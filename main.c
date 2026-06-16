#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
 
#include "auxiliary_files/processes.h"
#include "scheduler_algorithms/cfs.h"
#include "scheduler_algorithms/scheduler_manager.h"
#include "scheduler_algorithms/lottery.h"
#include "memory_algorithms/memory_manager.h"
 
#define INPUT_FILE "entradaEscalonador.txt"
#define OUTPUT_FILE "saidaEscalonador.txt"

// Executa o processo selecionado por um time slice (ou pelo tempo restante)
static void execute_process(int selected_idx, int *current_time, int *completed_processes) {
	// Imprime o evento de execução do processo selecionado
    Process *p = &processes[selected_idx];
    int run_time = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;
    print_process_event("RUN", *current_time, p, run_time);

    // Cada ciclo de CPU contabiliza um acesso real à memória
    for(int i = 0; i < run_time; i++) {
        if (p->next_access_index < p->page_sequence_len) {
            int page = p->page_sequence[p->next_access_index++];
            record_memory_access(p->pid, page); // Insere na fila de contexto global
        }
    }

	// Simula o avanço do tempo e a execução do processo	
    p->remaining_time -= run_time;
    *current_time += run_time;
 
    // Atualiza o vruntime para o CFS
    if (scheduler_manager_get_algorithm() == ALG_CFS) {
        p->vruntime += run_time * p->priority;
    }

	// Verifica se o processo terminou sua execução
    if (p->remaining_time <= 0) {
        p->is_completed = 1;
        p->completion_time = *current_time;
        (*completed_processes)++;
        print_process_event("FINISH", *current_time, p, 0);
        return;
    }
 
	// Anuncia que o processo foi preemptado
    print_process_event("PREEMPT", *current_time, p, 0);
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
	// Lê o arquivo de entrada e inicializa os processos
    read_input_file(INPUT_FILE);

    // Inicializa o gerenciador de escalonamento
	scheduler_manager_init();
 
    if (log_cfg.cpu_events) {
        log_printf("\n----------------------------------------------------------------------\n");
        log_printf("Log de escalonamento:\n");
        log_printf("Algoritmo: %s | Slice: %d\n\n", algorithm, time_slice);
    }
 
	// Loop principal de simulação do escalonamento
    int current_time = 0;
    int completed_processes = 0;
 
    while (completed_processes < num_processes) {
        announce_created_processes(current_time);
 
        int had_error = 0;
        int selected_idx = scheduler_manager_select_next(current_time, &had_error);
        if (had_error) return 1;
 
        if (selected_idx == -1) {
            if (log_cfg.cpu_events) log_printf("[T=%03d] IDLE\n", current_time);
            current_time++;
            continue;
        }
 
        execute_process(selected_idx, &current_time, &completed_processes);
 
		// Para o algoritmo de loteria, é necessário informar que o processo terminou para atualizar a segment tree
        if (scheduler_manager_get_algorithm() == ALG_LOTTERY) {
            lottery_process_finished(selected_idx);
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