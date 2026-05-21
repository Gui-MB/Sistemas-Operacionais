#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "auxiliary_files/processes.h"
#include "scheduler_algorithms/cfs.h"
#include "scheduler_algorithms/lottery.h"
#include "scheduler_algorithms/priority.h"
#include "scheduler_algorithms/round_robin.h"

#define INPUT_FILE "entradaEscalonador.txt"
#define OUTPUT_FILE "saidaEscalonador.txt"

// Verifica qual é o algoritmo selecionado
static int is_rr(void) {
	return strcmp(algorithm, "alternanciaCircular") == 0;
}
static int is_priority(void) {
	return strcmp(algorithm, "prioridade") == 0;
}
static int is_lottery(void) {
	return strcmp(algorithm, "loteria") == 0;
}
static int is_cfs(void) {
	return strcmp(algorithm, "CFS") == 0;
}

// Anuncia os processos criados no tempo atual e marca como anunciados para não repetir
static void announce_created_processes(int current_time) {
	for (int i = 0; i < num_processes; i++) {
		if (!processes[i].creation_announced && processes[i].creation_time <= current_time) {
			processes[i].creation_announced = 1;
			print_process_event("CREATE", current_time, &processes[i], 0);
		}
	}
}

// Função para selecionar o próximo processo a ser executado de acordo com o algoritmo escolhido
static int select_next_process(int current_time, int *had_error) {
	if (is_rr()) {
		return get_next_rr(current_time);
	}
	if (is_priority()) {
		return get_next_priority(current_time);
	}
	if (is_lottery()) {
		return get_next_lottery(current_time);
	}
	if (is_cfs()) {
		return get_next_cfs(current_time);
	}

	log_printf("Erro: Algoritmo desconhecido (%s)\n", algorithm);
	*had_error = 1;
	return -1;
}

// Função para executar o processo selecionado por um tempo determinado (time slice ou restante do processo)
static void execute_process(int selected_idx, int *current_time, int *completed_processes) {
	Process *p = &processes[selected_idx];
	int run_time = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;
	print_process_event("RUN", *current_time, p, run_time);

	// Atualiza o tempo restante do processo e o tempo atual	
	p->remaining_time -= run_time;
	*current_time += run_time;

	// Atualiza o vruntime do processo (apenas para CFS)
	if (is_cfs()) {
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
	
	// Reinsere o processo na estrutura de dados do CFS se for o algoritmo selecionado
	if (is_cfs()) {
		cfs_requeue(p);
	}
}

int main(void) {
	srand((unsigned)time(NULL));
	if (!init_output_file(OUTPUT_FILE)) {
		fprintf(stderr, "Erro ao abrir o arquivo de saida %s!\n", OUTPUT_FILE);
		return 1;
	}
	read_input_file(INPUT_FILE);

	log_printf("Inicio do escalonador\n");
	log_printf("Algoritmo: %s | Slice: %d\n\n", algorithm, time_slice);

	int current_time = 0;
	int completed_processes = 0;

	while (completed_processes < num_processes) {
		announce_created_processes(current_time);

		int had_error = 0;
		int selected_idx = select_next_process(current_time, &had_error);
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
	}

	// Destrói a estrutura de árvore no caso do CFS	
	if (is_cfs()) {
		cfs_destroy();
	}

	// Imprime a tabela com os resultados finais
	print_metrics();
	close_output_file();
	return 0;
}