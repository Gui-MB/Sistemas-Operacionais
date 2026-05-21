#include "round_robin.h"
#include "../Auxiliary_Files/processes.h"

// Variável estática para manter o índice do último processo escalonado
static int last_rr_index = -1;

// Seleciona o primeiro processo pronto e o coloca no fim da fila após executar.
int get_next_rr(int current_time) {
	
    int start = (last_rr_index + 1) % num_processes;
	
    for (int i = 0; i < num_processes; i++) {
        
        int idx = (start + i) % num_processes;
		
        if (processes[idx].creation_time <= current_time && !processes[idx].is_completed) {
			last_rr_index = idx;
			return idx;
        }
	}
	return -1;
}