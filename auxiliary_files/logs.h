#ifndef LOGS_H
#define LOGS_H

#include "processes.h"

// Estrutura de Configuração de Logs
typedef struct {
    int cpu_events; // Imprime eventos do escalonador (CREATE, RUN, PREEMPT, FINISH, IO_REQ, IO_DONE)
    int memory_steps; // Imprime o passo a passo da memória (Acertos, Trocas)
    int final_metrics; // Imprime as tabelas visuais de resultados no final do arquivo
} LogConfig;

extern LogConfig log_cfg; // Instância global configurável

int init_output_file(const char *filename);
void close_output_file(void);
int log_printf(const char *format, ...);

void print_process_event(const char *event, int current_time, const Process *p, int run_time);
void announce_created_processes(int current_time);
void print_metrics_scaling(void);
void print_metrics_memory(void);

// Imprime o estado completo do sistema (processo em execução, prontos, bloqueados e dispositivos).
// running_idx é o índice do processo em execução, ou -1 se a CPU estiver ociosa/sem processo definido.
void print_system_state(int current_time, int running_idx);

#endif
