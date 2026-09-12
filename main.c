#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
 
#include "auxiliary_files/processes.h"
#include "scheduler_algorithms/cfs.h"
#include "scheduler_algorithms/scheduler_manager.h"
#include "scheduler_algorithms/lottery.h"
#include "memory_algorithms/memory_manager.h"
#include "io_algorithms/io_manager.h"
 
#define INPUT_FILE "entradaEscalonador.txt"
#define OUTPUT_FILE "saidaEscalonador.txt"

// Inicia uma nova fatia de CPU para o processo selecionado, sorteando se e quando ele solicitará E/S
static int start_slice(int selected_idx, int current_time) {
    Process *p = &processes[selected_idx];
    int slice_length = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;

    // Contabiliza o tempo que o processo aguardou no estado pronto até começar a executar
    p->ready_wait_time += current_time - p->last_ready_entry_time;

    // Sorteia se o processo solicitará E/S e em que instante da fatia isso ocorrerá
    p->io_offset_ticks = -1;
    if (io_manager_should_request(p)) {
        p->io_offset_ticks = (rand() % slice_length) + 1;
        p->planned_device_id = io_manager_pick_device();
    }

    print_process_event("RUN", current_time, p, slice_length);
    print_system_state(current_time, selected_idx);
    return slice_length;
}

int main(void) {
    srand((unsigned)time(NULL));
 
    // Configuração de logs
    log_cfg.cpu_events = 1;
    log_cfg.memory_steps = 1;
    log_cfg.final_metrics = 1;

    if (!init_output_file(OUTPUT_FILE)) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída %s!\n", OUTPUT_FILE);
        return 1;
    }
	// Lê o arquivo de entrada e inicializa os processos e os dispositivos de E/S
    read_input_file(INPUT_FILE);

    // Inicializa o gerenciador de escalonamento
	scheduler_manager_init();
 
    if (log_cfg.cpu_events) {
        log_printf("\n----------------------------------------------------------------------\n");
        log_printf("Log de escalonamento:\n");
        log_printf("Algoritmo: %s | Slice: %d\n\n", algorithm, time_slice);
    }
 
	// Loop principal de simulação do escalonamento, executado ciclo a ciclo (1 unidade de tempo por iteração)
    int current_time = 0;
    int completed_processes = 0;
    int running_idx = -1;
    int slice_length = 0;
    int slice_ticks_done = 0;
 
    while (completed_processes < num_processes) {
        announce_created_processes(current_time);
 
        // Nenhum processo em execução no momento: seleciona o próximo processo pronto
        if (running_idx == -1) {
            int had_error = 0;
            running_idx = scheduler_manager_select_next(current_time, &had_error);
            if (had_error) return 1;
 
            if (running_idx == -1) {
                if (log_cfg.cpu_events) log_printf("[T=%03d] IDLE\n", current_time);
                io_manager_tick(current_time);
                current_time++;
                continue;
            }
 
            slice_length = start_slice(running_idx, current_time);
            slice_ticks_done = 0;
        }
 
        Process *p = &processes[running_idx];
 
        // Cada ciclo de CPU contabiliza um acesso real à memória
        if (p->next_access_index < p->page_sequence_len) {
            int page = p->page_sequence[p->next_access_index++];
            record_memory_access(p->pid, page); // Insere na fila de contexto global
        }
 
        p->remaining_time--;
        slice_ticks_done++;
 
        // Atualiza o vruntime para o CFS
        if (scheduler_manager_get_algorithm() == ALG_CFS) {
            p->vruntime += p->priority;
        }
 
        current_time++;
        io_manager_tick(current_time);
 
        // Verifica se o processo terminou sua execução
        if (p->remaining_time <= 0) {
            p->is_completed = 1;
            p->completion_time = current_time;
            completed_processes++;
            print_process_event("FINISH", current_time, p, 0);

            // Para o algoritmo de loteria, é necessário informar que o processo terminou para atualizar a segment tree
            if (scheduler_manager_get_algorithm() == ALG_LOTTERY) {
                lottery_process_finished(running_idx);
            }

            running_idx = -1;
            print_system_state(current_time, -1);
            continue;
        }
 
        // Verifica se chegou o instante sorteado para a solicitação de E/S
        if (p->io_offset_ticks == slice_ticks_done) {
            io_manager_request(running_idx, p->planned_device_id, current_time);

            if (scheduler_manager_get_algorithm() == ALG_LOTTERY) {
                lottery_process_finished(running_idx);
            }

            running_idx = -1;
            print_system_state(current_time, -1);
            continue;
        }
 
        // Verifica se a fatia de CPU foi consumida por completo (preempção)
        if (slice_ticks_done >= slice_length) {
            p->last_ready_entry_time = current_time;
            print_process_event("PREEMPT", current_time, p, 0);

            if (scheduler_manager_get_algorithm() == ALG_LOTTERY) {
                lottery_process_finished(running_idx);
            }

            running_idx = -1;
            print_system_state(current_time, -1);
            continue;
        }
    }

	// Destrói a estrutura de árvore no caso do CFS	
    if (scheduler_manager_get_algorithm() == ALG_CFS) {
        cfs_destroy();
    }

    print_metrics_scaling();
    print_metrics_memory();
    
    close_output_file();
    return 0;
}