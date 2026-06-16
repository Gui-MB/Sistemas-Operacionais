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

    // Atualiza o tempo restante do processo e o tempo atual	
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
 
    if (!init_output_file(OUTPUT_FILE)) {
        fprintf(stderr, "Erro ao abrir o arquivo de saida %s!\n", OUTPUT_FILE);
        return 1;
    }
 
    read_input_file(INPUT_FILE);

    scheduler_manager_init();
 
    log_printf("Inicio do escalonador\n");
    log_printf("Algoritmo: %s | Slice: %d\n\n", algorithm, time_slice);
 
    //Simulação de escalonamento
    int current_time = 0;
    int completed_processes = 0;
 
    // Substituição de páginas
    int total_fifo    = 0;
    int total_lru     = 0;
    int total_nfu     = 0;
    int total_optimal = 0;

    while (completed_processes < num_processes) {
        announce_created_processes(current_time);
 
        int had_error = 0;
        int selected_idx = scheduler_manager_select_next(current_time, &had_error);
        if (had_error) {
            return 1;
        }
 
        // CPU ociosa quando não há processos prontos
        if (selected_idx == -1) {
            log_printf("[T=%03d] IDLE\n", current_time);
            current_time++;
            continue;
        }
 
        execute_process(selected_idx, &current_time, &completed_processes);
 
		// Para o algoritmo de loteria, é necessário informar que o processo terminou para atualizar a segment tree
        if (scheduler_manager_get_algorithm() == ALG_LOTTERY) {
            lottery_process_finished(selected_idx);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        PageFaultResult res;
        memory_manager_simulate(&processes[i], &res);
 
        total_fifo    += res.fifo_faults;
        total_lru     += res.lru_faults;
        total_nfu     += res.nfu_faults;
        total_optimal += res.optimal_faults;
    }
 
	// Imprime a linha de resultados de memória
    log_printf("\n--- RESULTADOS DA MEMÓRIA ---\n");
    log_printf("FIFO|LRU|NFU|OTM|Melhor\n");
    log_printf("%d|%d|%d|%d|", total_fifo, total_lru, total_nfu, total_optimal);
 
    /* Determina o algoritmo não-ótimo mais próximo do ótimo */
    int diff_fifo = abs(total_fifo - total_optimal);
    int diff_lru  = abs(total_lru  - total_optimal);
    int diff_nfu  = abs(total_nfu  - total_optimal);
 
    const char *best      = "FIFO";
    int         best_diff = diff_fifo;
    if (diff_lru < best_diff) { best = "LRU"; best_diff = diff_lru; }
    if (diff_nfu < best_diff) { best = "NFU"; }
 
    log_printf("%s\n", best);

	// Destrói a estrutura de árvore no caso do CFS	
    if (scheduler_manager_get_algorithm() == ALG_CFS) {
        cfs_destroy();
    }

    // Imprime a tabela com os resultados finais para o escalonamento
    print_metrics_scaling();

    // Imprime a tabela com os resultados finais para a memória
    print_metrics_memory();
    close_output_file();
    return 0;
}