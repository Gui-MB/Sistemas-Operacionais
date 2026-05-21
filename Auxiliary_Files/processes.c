#include "processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Process processes[MAX_PROCESSES];
int num_processes = 0;
int time_slice = 0;
char algorithm[50];

// Le o arquivo de entrada e inicializa a lista de processos.
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

        // Inicializa os campos restantes do processo: remaining_time, vruntime, is_completed
        processes[num_processes].remaining_time = processes[num_processes].exec_time;
        processes[num_processes].vruntime = 0;
        processes[num_processes].is_completed = 0;
        num_processes++;
    }
    fclose(file);
}

// Imprime a tabela de resultados finais para cada processo.
void print_metrics(void) {
    printf("\n--- RESULTADOS FINAIS ---\n");
    printf("%-5s | %-16s\n", "PID", "Latência");
    printf("-------------------------------------------------\n");

    float total_wt = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        int turnaround_time = p.completion_time - p.creation_time;
        int waiting_time = turnaround_time - p.exec_time;

        total_wt += waiting_time;

        printf("%-5d | %-16d\n", p.pid, waiting_time);
    }

    printf("-------------------------------------------------\n");
    printf("Média | %-16.2f\n", total_wt / num_processes);
}