#ifndef LOGS_H
#define LOGS_H

#include "processes.h"

// Estrutura de Configuração de Logs
typedef struct {
    int cpu_events;     // Imprime eventos do escalonador (CREATE, RUN, PREEMPT, FINISH)
    int memory_steps;   // Imprime o passo a passo da memória (Acertos, Trocas)
    int final_metrics;  // Imprime as tabelas visuais de resultados no final do arquivo
} LogConfig;

extern LogConfig log_cfg; // Instância global configurável

int init_output_file(const char *filename);
void close_output_file(void);
int log_printf(const char *format, ...);

void print_process_event(const char *event, int current_time, const Process *p, int run_time);
void announce_created_processes(int current_time);
void print_metrics_scaling(void);
void print_metrics_memory(void);

#endif