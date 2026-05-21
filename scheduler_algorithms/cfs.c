#include "cfs.h"
#include <stddef.h>
#include "../auxiliary_files/processes.h"
#include "../auxiliary_files/red_and_black_tree.h"

static int cfs_initialized = 0;

// Inicializa a estrutura de dados do CFS
void cfs_init(void) {
	if (!cfs_initialized) {
		rb_init();
		cfs_initialized = 1;
	}
}

// Destrói a árvore
void cfs_destroy(void) {
	if (cfs_initialized) {
		rb_destroy();
		cfs_initialized = 0;
	}
}

// Enfileira novos processos prontos na árvore
static void cfs_enqueue_ready(int current_time) {
	for (int i = 0; i < num_processes; i++) {
		if (processes[i].creation_time <= current_time && !processes[i].is_completed && !processes[i].in_cfs_tree) {
			rb_insert(&processes[i]);
			processes[i].in_cfs_tree = 1;
		}
	}
}

// Seleciona o processo pronto com menor vruntime e enfileira novos processos na árvore
int get_next_cfs(int current_time) {
	cfs_init();
	cfs_enqueue_ready(current_time);

	// Encontra o processo com menor vruntime e o remove da árvore
	Process *selected = rb_pop_min_process();
	if (selected == NULL) {
		return -1;
	}

	selected->in_cfs_tree = 0;
	return (int)(selected - processes);
}