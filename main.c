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

int main(void) {
	srand((unsigned)time(NULL));
	read_input_file(INPUT_FILE);

	printf("Inicio do escalonador\n");
	printf("Algoritmo: %s | Slice: %d\n\n", algorithm, time_slice);

	int current_time = 0;
	int completed_processes = 0;

	while (completed_processes < num_processes) {
		// Anúncio de criação dos processos
		for (int i = 0; i < num_processes; i++) {
			if (!processes[i].creation_announced && processes[i].creation_time <= current_time) {
				processes[i].creation_announced = 1;
				print_process_event("CREATE", current_time, &processes[i], 0);
			}
		}

		int selected_idx = -1;

		// Seleção do próximo processo a executar com base no algoritmo escolhido
		if (strcmp(algorithm, "alternanciaCircular") == 0) {
			selected_idx = get_next_rr(current_time);
		} else if (strcmp(algorithm, "prioridade") == 0) {
			selected_idx = get_next_priority(current_time);
		} else if (strcmp(algorithm, "loteria") == 0) {
			selected_idx = get_next_lottery(current_time);
		} else if (strcmp(algorithm, "CFS") == 0) {
			selected_idx = get_next_cfs(current_time);
		} else {
			printf("Erro: Algoritmo desconhecido (%s)\n", algorithm);
			return 1;
		}

		// CPU ociosa quando não há processos prontos
		if (selected_idx == -1) {
			printf("[T=%03d] IDLE\n", current_time);
			current_time++;
			continue;
		}

		// Processo selecionado para execução				
		Process *p = &processes[selected_idx];

		// Define quanto tempo o processo ficará na CPU neste ciclo
		int run_time = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;
		print_process_event("RUN", current_time, p, run_time);

		// Atualiza o tempo restante do processo e o tempo atual do sistema		
		p->remaining_time -= run_time;
		if (strcmp(algorithm, "CFS") == 0) {
			p->vruntime += run_time * p->priority;
		}
		current_time += run_time;

		// Verifica se o processo foi finalizado ou se deve ser preemptado		
		if (p->remaining_time <= 0) {
			p->is_completed = 1;
			p->completion_time = current_time;
			completed_processes++;
			print_process_event("FINISH", current_time, p, 0);
		} else {
			print_process_event("PREEMPT", current_time, p, 0);
		}
	}

	print_metrics();
	return 0;
}