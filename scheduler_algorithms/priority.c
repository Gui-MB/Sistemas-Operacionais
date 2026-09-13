#include "priority.h"

#include <stddef.h>

#include "../processes/process_manager.h"
#include "../auxiliary_files/heap_min.h"

static int priority_initialized = 0;

// Inicializa a estrutura de dados de prioridade
static void priority_init(void) {
	if (!priority_initialized) {
		heap_min_init();
		priority_initialized = 1;
	}
}

// Enfileira processos prontos no heap
static void priority_enqueue_ready(int current_time) {
	for (int i = 0; i < num_processes; i++) {
		if (processes[i].creation_time <= current_time && !processes[i].is_completed && !processes[i].in_priority_heap) {
			heap_min_insert(&processes[i]);
			processes[i].in_priority_heap = 1;
		}
	}
}

// Seleciona o processo pronto com menor prioridade. Em empate, menor PID vence.
int get_next_priority(int current_time) {
	priority_init();
	priority_enqueue_ready(current_time);

	Process *selected = heap_min_pop();
	if (selected == NULL) {
		return -1;
	}

	selected->in_priority_heap = 0;
	return (int)(selected - processes);
}