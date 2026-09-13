#ifndef SCHEDULER_MANAGER_H
#define SCHEDULER_MANAGER_H

typedef enum {
    ALG_RR,
    ALG_PRIORITY,
    ALG_LOTTERY,
    ALG_CFS,
    ALG_UNKNOWN
} Algorithm;

void scheduler_manager_init(void);
void scheduler_manager_destroy(void);
Algorithm scheduler_manager_get_algorithm(void);
int scheduler_manager_select_next(int current_time, int *had_error);
void scheduler_manager_process_finished(int pid);

#endif