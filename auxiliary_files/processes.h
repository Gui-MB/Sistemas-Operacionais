#ifndef PROCESSES_H
#define PROCESSES_H

#define MAX_PROCESSES 1000

// Estrutura para armazenar as informacoes de cada processo
typedef struct {
    int pid;
    int creation_time;
    int exec_time;
    int remaining_time;
    int priority;
    int vruntime;
    int completion_time;
    int is_completed;
    int creation_announced;
    int in_cfs_tree;
} Process;

extern Process processes[MAX_PROCESSES];
extern int num_processes;
extern int time_slice;
extern char algorithm[50];

void read_input_file(const char *filename);
void print_process_event(const char *event, int current_time, const Process *p, int run_time);
void print_metrics(void);

#endif