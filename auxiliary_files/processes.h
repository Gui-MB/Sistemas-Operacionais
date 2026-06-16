#ifndef PROCESSES_H
#define PROCESSES_H

#define MAX_PROCESSES 1000

// Estrutura para armazenar as informacoes de cada processo
typedef struct {
    int pid;
    int creation_time;
    int exec_time;
    int remaining_time;
    int completion_time;
    int priority;
    int vruntime;
    int in_cfs_tree;
    int in_priority_heap;
    int is_completed;
    int creation_announced;
    int memory_bytes;
    int virtual_pages;
    int frame_limit;
    int page_sequence_len;
    int next_access_index;
    int *page_sequence;
    int fifo_faults;
    int lru_faults;
    int nfu_faults;
    int optimal_faults;
} Process;

extern Process processes[MAX_PROCESSES];

extern int num_processes;
extern int time_slice;
extern char algorithm[50];
extern char memory_policy[50];
extern int memory_size_bytes;
extern int page_size_bytes;
extern int allocation_percent;

int init_output_file(const char *filename);
void close_output_file(void);
int log_printf(const char *format, ...);
void read_input_file(const char *filename);
void free_input_data(void);
void print_process_event(const char *event, int current_time, const Process *p, int run_time);
void announce_created_processes(int current_time);
void print_metrics_scaling(void);
void print_metrics_memory(void);

#endif