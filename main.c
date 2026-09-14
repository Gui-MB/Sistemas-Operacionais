#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "processes/process_manager.h"
#include "scheduler_algorithms/scheduler_manager.h"
#include "memory_algorithms/memory_manager.h"
#include "io_algorithms/io_manager.h"
#include "logs/log_manager.h"

#define INPUT_FILE "entradaEscalonador.txt"
#define OUTPUT_FILE "saidaEscalonador.txt"

// Funções Auxiliares de Configuração e Finalização
static int init_simulation(void) {
    srand((unsigned)time(NULL));

    if (!init_output_file(OUTPUT_FILE)) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída %s!\n", OUTPUT_FILE);
        return 0;
    }

    read_input_file(INPUT_FILE);
    scheduler_manager_init();

    if (log_cfg.cpu_events) {
        log_printf("Log de Escalonamento:\n");
        log_printf("Algoritmo: %s | Slice: %d\n\n", algorithm, time_slice);
    }
    
    return 1;
}

// Função para finalizar a simulação, liberando recursos e imprimindo métricas
static void finish_simulation(void) {
    scheduler_manager_destroy();

    print_metrics_scaling();
    print_metrics_memory();
    print_metrics_io();
    close_output_file();
}

// Gerenciamento de Processos e CPU
// Inicia uma nova fatia de CPU para o processo selecionado
static int start_slice(int selected_idx, int current_time) {
    Process *p = &processes[selected_idx];
    int slice_length = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;

    // Contabiliza tempo de espera no estado pronto
    p->ready_wait_time += current_time - p->last_ready_entry_time;
    p->io_offset_ticks = -1;

    // Sorteia se/quando haverá solicitação de E/S
    if (io_manager_should_request(p)) {
        p->io_offset_ticks = (rand() % slice_length) + 1;
        p->planned_device_id = io_manager_pick_device();
    }

    print_process_event("RUN", current_time, p, slice_length);
    print_system_state(current_time, selected_idx);
    
    return slice_length;
}

// Libera a CPU e notifica estruturas que dependem da saída do processo
static void release_cpu(int *running_idx, int current_time) {
    scheduler_manager_process_finished(*running_idx);
    
    *running_idx = -1;
    print_system_state(current_time, -1);
}

// Executa um ciclo de memória e CPU para o processo em andamento
static void execute_process_tick(Process *p) {
    // Ciclo de CPU contabiliza um acesso real à memória
    if (p->next_access_index < p->page_sequence_len) {
        int page = p->page_sequence[p->next_access_index++];
        record_memory_access(p->pid, page); 
    }

    p->remaining_time--;

    if (scheduler_manager_get_algorithm() == ALG_CFS) {
        p->vruntime += p->priority;
    }
}

// Loop Principal
int main(void) {
    if (!init_simulation()) {
        return 1;
    }

    int current_time = 0;
    int completed_processes = 0;
    int running_idx = -1;
    
    int slice_length = 0;
    int slice_ticks_done = 0;

    while (completed_processes < num_processes) {
        announce_created_processes(current_time);

        // 1. Seleção de Processo (se a CPU estiver ociosa)
        if (running_idx == -1) {
            int had_error = 0;
            running_idx = scheduler_manager_select_next(current_time, &had_error);
            
            if (had_error) return 1;

            if (running_idx == -1) { // Sistema IDLE
                if (log_cfg.cpu_events) log_printf("\n[T=%04d] IDLE\n", current_time);
                current_time++;
                io_manager_tick(current_time);
                continue;
            }

            slice_length = start_slice(running_idx, current_time);
            slice_ticks_done = 0;
        }

        // 2. Execução do Ciclo Atual
        Process *p = &processes[running_idx];
        execute_process_tick(p);
        slice_ticks_done++;
        
        current_time++;
        io_manager_tick(current_time);

        // 3. Verificação de Estado Pós-Ciclo (Fim, E/S ou Preempção)
        if (p->remaining_time <= 0) {
            p->is_completed = 1;
            p->completion_time = current_time;
            completed_processes++;
            
            print_process_event("FINISH", current_time, p, 0);
            release_cpu(&running_idx, current_time);
        } 
        else if (p->io_offset_ticks == slice_ticks_done) {
            io_manager_request(running_idx, p->planned_device_id, current_time);
            release_cpu(&running_idx, current_time);
        } 
        else if (slice_ticks_done >= slice_length) {
            p->last_ready_entry_time = current_time;
            print_process_event("PREEMPT", current_time, p, 0);
            release_cpu(&running_idx, current_time);
        }
    }

    finish_simulation();
    return 0;
}