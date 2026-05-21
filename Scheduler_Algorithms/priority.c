#include "priority.h"
#include "../Auxiliary_Files/processes.h"

// Seleciona o processo com maior prioridade.
int get_next_priority(int current_time) {
	int best_idx = -1;
	int max_prio = -1;

	for (int i = 0; i < num_processes; i++) {
		if (processes[i].creation_time <= current_time && !processes[i].is_completed) {
			if (processes[i].priority > max_prio) {
				max_prio = processes[i].priority;
				best_idx = i;
			}
		}
	}
	return best_idx;
}