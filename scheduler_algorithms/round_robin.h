#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

int get_next_rr(int current_time);

// Reenfileira, ao final da fila de prontos, um processo que acabou de ser liberado por um dispositivo de E/S
void rr_enqueue_unblocked(int process_idx);

#endif
