#include "cfs.h"
#include <stddef.h>
#include "../Auxiliary_Files/processes.h"
#include "../Auxiliary_Files/red_and_black_tree.h"

// Seleciona o processo pronto com menor vruntime.
int get_next_cfs(int current_time) {
	rb_init();

	for (int i = 0; i < num_processes; i++) {
		if (processes[i].creation_time <= current_time && !processes[i].is_completed) {
			rb_insert(&processes[i]);
		}
	}

	Process *selected = rb_minimum_process();
	int best_idx = -1;

	if (selected != NULL) {
		best_idx = (int)(selected - processes);
	}

	rb_destroy();
	return best_idx;
}