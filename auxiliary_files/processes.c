#include "processes.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Process processes[MAX_PROCESSES];
int num_processes = 0;
int time_slice = 0;
char algorithm[50];
static FILE *log_file = NULL;

int init_output_file(const char *filename) {
    log_file = fopen(filename, "w");
    if (!log_file) {
        return 0;
    }
    return 1;
}

void close_output_file(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

int log_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int count_stdout = vfprintf(stdout, format, args);
    va_end(args);

    if (log_file) {
        va_list args_file;
        va_start(args_file, format);
        vfprintf(log_file, format, args_file);
        va_end(args_file);
        fflush(log_file);
    }

    fflush(stdout);
    return count_stdout;
}

// Le o arquivo de entrada e inicializa a lista de processos, um por vez.
void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_printf("Erro ao abrir o arquivo %s!\n", filename);
        exit(1);
    }

    char line[256];

    // Primeira linha: algoritmo|time_slice
    if (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^|]|%d", algorithm, &time_slice);
        algorithm[strcspn(algorithm, "\r\n")] = 0;
    }

    // Processos: tempo_criacao|pid|tempo_execucao|prioridade
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d|%d|%d|%d",
               &processes[num_processes].creation_time,
               &processes[num_processes].pid,
               &processes[num_processes].exec_time,
               &processes[num_processes].priority);

        // Inicializa os campos restantes do processo: remaining_time, vruntime, is_completed, creation_announced, in_cfs_tree
        processes[num_processes].remaining_time = processes[num_processes].exec_time;
        processes[num_processes].vruntime = 0;
        processes[num_processes].is_completed = 0;
        processes[num_processes].creation_announced = 0;
        processes[num_processes].in_cfs_tree = 0;
        num_processes++;
    }
    fclose(file);
}

// Imprime os detalhes de acordo com o algoritmo selecionado
static void print_algo_details(const Process *p) {
    if (strcmp(algorithm, "prioridade") == 0) {
        log_printf(" | priority=%-4d", p->priority);
    } else if (strcmp(algorithm, "loteria") == 0) {
        log_printf(" | tickets=%-4d", p->priority);
    } else if (strcmp(algorithm, "CFS") == 0) {
        log_printf(" | priority=%-4d | vruntime=%-4d", p->priority, p->vruntime);
    }
}

// Imprime os eventos de criação, execução, preempção e finalização dos processos.
void print_process_event(const char *event, int current_time, const Process *p, int run_time) {
    log_printf("[T=%03d] %-7s | pid=%-3d", current_time, event, p->pid);

    if (strcmp(event, "CREATE") == 0) {
        log_printf(" | total_t=%-8d", p->exec_time);
    } else if (strcmp(event, "RUN") == 0) {
        log_printf(" | remaining_t=%-4d | cpu_slice=%-3d", p->remaining_time, run_time);
    } else if (strcmp(event, "PREEMPT") == 0) {
        log_printf(" | remaining_t=%-4d", p->remaining_time);
    }

    print_algo_details(p);
    log_printf("\n");
}

// Imprime a tabela de resultados finais para cada processo.
void print_metrics(void) {
    log_printf("\n--- RESULTADOS DA EXECUÇÃO ---\n");
    log_printf("%-5s | %-17s | %-16s | %-16s\n", "PID", "Latência", "Tempo de Espera", "Tempo de Execução");
    log_printf("---------------------------------------------------------------------\n");

    float total_latency = 0;
    float total_wt = 0;
    int total_exec_time = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        int latency_time = p.completion_time - p.creation_time;
        int waiting_time = latency_time - p.exec_time;

        total_latency += latency_time;
        total_wt += waiting_time;
        total_exec_time += p.exec_time;

        log_printf("%-5d | %-16d | %-16d | %-16d\n", p.pid, latency_time, waiting_time, p.exec_time);
    }

    log_printf("---------------------------------------------------------------------\n");
    log_printf("Latência Média: %-16.2f\n", total_latency / num_processes);
    log_printf("Tempo de Espera Médio: %-16.2f\n", total_wt / num_processes);
    log_printf("Tempo Total de Execução: %-16d\n", total_exec_time);
}