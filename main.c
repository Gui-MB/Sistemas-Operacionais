#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "auxiliary_files/processes.h"
#include "scheduler_algorithms/cfs.h"
#include "scheduler_algorithms/scheduler_manager.h"
#include "scheduler_algorithms/lottery.h"

#define INPUT_FILE "entradaEscalonador.txt"
#define OUTPUT_FILE "saidaEscalonador.txt"

// Função para executar o processo selecionado por um tempo determinado (time slice ou restante do processo)
static void execute_process(int selected_idx, int *current_time, int *completed_processes) {
	// Imprime o evento de execução do processo selecionado
	Process *p = &processes[selected_idx];
	int run_time = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;
	print_process_event("RUN", *current_time, p, run_time);

	// Atualiza o tempo restante do processo e o tempo atual	
	p->remaining_time -= run_time;
	*current_time += run_time;

	// Atualiza o vruntime do processo para o CFS
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

	int current_time = 0;
	int completed_processes = 0;

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

	// Destrói a estrutura de árvore no caso do CFS	
	if (scheduler_manager_get_algorithm() == ALG_CFS) {
		cfs_destroy();
	}

	// Imprime a tabela com os resultados finais
	print_metrics();
	close_output_file();
	return 0;
}