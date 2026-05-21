#include "lottery.h"
#include <stdlib.h>
#include "../Auxiliary_Files/processes.h"

// Sorteia um processo proporcional aos seus bilhetes (prioridade).
int get_next_lottery(int current_time) {
	int total_tickets = 0;

	for (int i = 0; i < num_processes; i++) {
		if (processes[i].creation_time <= current_time && !processes[i].is_completed) {
			total_tickets += processes[i].priority;
		}
	}

	if (total_tickets == 0) {
		return -1;
	}

	int winning_ticket = rand() % total_tickets;
	int current_sum = 0;

	for (int i = 0; i < num_processes; i++) {
		if (processes[i].creation_time <= current_time && !processes[i].is_completed) {
			current_sum += processes[i].priority;
			if (current_sum > winning_ticket) {
				return i;
			}
		}
	}

	return -1;
}