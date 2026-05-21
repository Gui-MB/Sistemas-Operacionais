#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Auxiliary_Files/processes.h"
#include "Scheduler_Algorithms/cfs.h"
#include "Scheduler_Algorithms/lottery.h"
#include "Scheduler_Algorithms/priority.h"
#include "Scheduler_Algorithms/round_robin.h"

#define INPUT_FILE "Input_Files/entradaEscalonador.txt"

int main(void) {
	srand((unsigned)time(NULL));
	read_input_file(INPUT_FILE);

	printf("Iniciando escalonador...\n");
	printf("Algoritmo: %s | Fatia de Tempo (Slice): %d\n\n", algorithm, time_slice);

	int current_time = 0;
	int completed_processes = 0;

	while (completed_processes < num_processes) {
		int selected_idx = -1;

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
			current_time++;
			continue;
		}

		Process *p = &processes[selected_idx];

		// Define quanto tempo o processo ficará na CPU neste ciclo
		int run_time = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;

		printf("[Tempo %03d] Processo PID %d entrou na CPU | Faltavam: %d unid.\n",
			   current_time, p->pid, p->remaining_time);

		p->remaining_time -= run_time;
		p->vruntime += run_time;
		current_time += run_time;

		if (p->remaining_time <= 0) {
			p->is_completed = 1;
			p->completion_time = current_time;
			completed_processes++;
			printf("-> Processo PID %d CONCLUIDO no tempo %d\n", p->pid, current_time);
		} else {
			printf("> Processo PID %d SAIU da CPU | Restam: %d unid.\n", p->pid, p->remaining_time);
		}
	}

	print_metrics();
	return 0;
}