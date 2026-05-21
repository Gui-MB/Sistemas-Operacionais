#include "processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Process processes[MAX_PROCESSES];
int num_processes = 0;
int time_slice = 0;
char algorithm[50];

// Le o arquivo de entrada e inicializa a lista de processos, um por vez.
void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo %s!\n", filename);
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

        // Inicializa os campos restantes do processo: remaining_time, vruntime, is_completed, creation_announced
        processes[num_processes].remaining_time = processes[num_processes].exec_time;
        processes[num_processes].vruntime = 0;
        processes[num_processes].is_completed = 0;
        processes[num_processes].creation_announced = 0;
        num_processes++;
    }
    fclose(file);
}

// Imprime os detalhes de acordo com o algoritmo selecionado
static void print_algo_details(const Process *p) {
    if (strcmp(algorithm, "prioridade") == 0) {
        printf(" | priority=%d", p->priority);
    } else if (strcmp(algorithm, "loteria") == 0) {
        printf(" | tickets=%d", p->priority);
    } else if (strcmp(algorithm, "CFS") == 0) {
        printf(" | priority=%d | vruntime=%d", p->priority, p->vruntime);
    }
}

// Imprime os eventos de criação, execução, preempção e finalização dos processos.
void print_process_event(const char *event, int current_time, const Process *p, int run_time) {
    printf("[T=%03d] %s pid=%d", current_time, event, p->pid);

    if (strcmp(event, "CREATE") == 0) {
        printf(" | total_exec_time=%d", p->exec_time);
    } else if (strcmp(event, "RUN") == 0) {
        printf(" | remaining_exec_time=%d | CPU_slice=%d", p->remaining_time, run_time);
    } else if (strcmp(event, "PREEMPT") == 0) {
        printf(" | remaining_exec_time=%d", p->remaining_time);
    }

    print_algo_details(p);
    printf("\n");
}

// Imprime a tabela de resultados finais para cada processo.
void print_metrics(void) {
    printf("\n--- RESULTADOS DA EXECUÇÃO ---\n");
    printf("%-5s | %-16s | %-16s\n", "PID", "Latência", "Tempo de Espera");
    printf("-------------------------------------------------\n");

    float total_latency = 0;
    float total_wt = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        int latency_time = p.completion_time - p.creation_time;
        int waiting_time = latency_time - p.exec_time;

        total_latency += latency_time;
        total_wt += waiting_time;

        printf("%-5d | %-16d | %-16d\n", p.pid, latency_time, waiting_time);
    }

    printf("-------------------------------------------------\n");
    printf("Latência Média: | %-16.2f\nTempo de Espera Médio:| %-16.2f\n", total_latency / num_processes, total_wt / num_processes);
}