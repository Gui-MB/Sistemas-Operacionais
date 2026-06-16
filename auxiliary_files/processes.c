#include "processes.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../memory_algorithms/memory_manager.h"

Process processes[MAX_PROCESSES];
int num_processes = 0;
int time_slice = 0;
char algorithm[50];
char memory_policy[50];
int memory_size_bytes = 0;
int page_size_bytes = 0;
int allocation_percent = 0;
static FILE *log_file = NULL;

// Imprime mensagens no arquivo de saída
int init_output_file(const char *filename) {
    log_file = fopen(filename, "w");
    if (!log_file) {
        return 0;
    }
    return 1;
}

// Fecha o arquivo de saída, se estiver aberto
void close_output_file(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

// Imprime mensagens tanto no console quanto no arquivo de saída
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

// Conta o número de páginas na sequência de acesso de um processo
static int count_pages_in_sequence(const char *sequence) {
    int count = 0;
    char *copy = strdup(sequence);
    if (!copy) {
        return 0;
    }

    char *save_ptr = NULL;
    for (char *token = strtok_r(copy, " \t\r\n", &save_ptr); token != NULL; token = strtok_r(NULL, " \t\r\n", &save_ptr)) {
        count++;
    }

    free(copy);
    return count;
}

// Libera a memória alocada para as sequências de acesso dos processos
void free_input_data(void) {
    for (int i = 0; i < num_processes; i++) {
        free(processes[i].page_sequence);
        processes[i].page_sequence = NULL;
        processes[i].page_sequence_len = 0;
    }
    num_processes = 0;
}

// Lê o arquivo de entrada e inicializa a lista de processos
void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_printf("Erro ao abrir o arquivo %s!\n", filename);
        exit(1);
    }

    free_input_data();

    char *line = NULL;
    size_t line_size = 0;

    // Primeira linha: algoritmo|fraçãoCPU|políticaMemória|tamanhoMemória|tamanhoPáginasMolduras|percentualAlocação
    if (getline(&line, &line_size, file) != -1) {
        sscanf(line, "%49[^|]|%d|%49[^|]|%d|%d|%d",
               algorithm,
               &time_slice,
               memory_policy,
               &memory_size_bytes,
               &page_size_bytes,
               &allocation_percent);

        algorithm[strcspn(algorithm, "\r\n")] = 0;
        memory_policy[strcspn(memory_policy, "\r\n")] = 0;
    }

    // Linhas dos processos: tempo_criacao|pid|tempo_execucao|prioridade|qtdeMemoria|sequênciaAcesso
    while (getline(&line, &line_size, file) != -1) {
        char *save_ptr = NULL;
        char *creation = strtok_r(line, "|", &save_ptr);
        char *pid = strtok_r(NULL, "|", &save_ptr);
        char *exec_time = strtok_r(NULL, "|", &save_ptr);
        char *priority = strtok_r(NULL, "|", &save_ptr);
        char *memory = strtok_r(NULL, "|", &save_ptr);
        char *sequence = strtok_r(NULL, "", &save_ptr);

        if (!creation || !pid || !exec_time || !priority || !memory || !sequence) {
            continue;
        }

        Process *process = &processes[num_processes];
        memset(process, 0, sizeof(*process));

        process->creation_time = (int)strtol(creation, NULL, 10);
        process->pid = (int)strtol(pid, NULL, 10);
        process->exec_time = (int)strtol(exec_time, NULL, 10);
        process->priority = (int)strtol(priority, NULL, 10);
        process->memory_bytes = (int)strtol(memory, NULL, 10);
        process->page_sequence_len = count_pages_in_sequence(sequence);
        process->virtual_pages = (process->memory_bytes + page_size_bytes - 1) / page_size_bytes;
        if (process->virtual_pages <= 0) {
            process->virtual_pages = 1;
        }
        process->frame_limit = (process->virtual_pages * allocation_percent + 99) / 100;
        if (process->frame_limit <= 0) {
            process->frame_limit = 1;
        }
        if (process->frame_limit > process->virtual_pages) {
            process->frame_limit = process->virtual_pages;
        }

        process->page_sequence = calloc((size_t)process->page_sequence_len, sizeof(int));
        if (!process->page_sequence && process->page_sequence_len > 0) {
            fclose(file);
            free(line);
            log_printf("Erro de memoria ao ler a sequência de páginas do processo %d!\n", process->pid);
            exit(1);
        }

        char *seq_copy = strdup(sequence);
        if (!seq_copy) {
            fclose(file);
            free(line);
            log_printf("Erro de memoria ao copiar a sequência de páginas do processo %d!\n", process->pid);
            exit(1);
        }

        int page_idx = 0;
        char *seq_save_ptr = NULL;
        for (char *token = strtok_r(seq_copy, " \t\r\n", &seq_save_ptr); token != NULL; token = strtok_r(NULL, " \t\r\n", &seq_save_ptr)) {
            if (page_idx < process->page_sequence_len) {
                process->page_sequence[page_idx++] = (int)strtol(token, NULL, 10);
            }
        }

        free(seq_copy);

        // Inicializa campos extras para cada processo: remaining_time|vruntime|is_completed|creation_announced|in_cfs_tree|in_priority_heap|next_access_index
        process->remaining_time = process->exec_time;
        process->vruntime = 0;
        process->completion_time = 0;
        process->is_completed = 0;
        process->creation_announced = 0;
        process->in_cfs_tree = 0;
        process->in_priority_heap = 0;
        process->next_access_index = 0;

        num_processes++;
    }

    free(line);
    fclose(file);
}

// Imprime os detalhes de evento de acordo com o algoritmo selecionado
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
    } else if (strcmp(event, "FINISH") == 0) {
        log_printf(" | remaining_t=%-4d", p->remaining_time);
    }

    print_algo_details(p);
    log_printf("\n");
}

// Anuncia os processos criados no tempo atual e marca como anunciados
void announce_created_processes(int current_time) {
    for (int i = 0; i < num_processes; i++) {
        if (!processes[i].creation_announced && processes[i].creation_time <= current_time) {
            processes[i].creation_announced = 1;
            print_process_event("CREATE", current_time, &processes[i], 0);
        }
    }
}

// Imprime a tabela de resultados finais do escalonamento
void print_metrics_scaling(void) {
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

// Imprime a tabela de resultados finais do gerenciamento de memória
void print_metrics_memory(void) {
int total_fifo    = 0;
    int total_lru     = 0;
    int total_nfu     = 0;
    int total_optimal = 0;

    for (int i = 0; i < num_processes; i++) {
        PageFaultResult res;
        memory_manager_simulate(&processes[i], &res);

        total_fifo    += res.fifo_faults;
        total_lru     += res.lru_faults;
        total_nfu     += res.nfu_faults;
        total_optimal += res.optimal_faults;
    }

    // Imprime a linha de resultados de memória
    log_printf("\n--- RESULTADOS DA MEMÓRIA ---\n");
    log_printf("FIFO|LRU|NFU|OTM|Melhor\n");
    log_printf("%d|%d|%d|%d|", total_fifo, total_lru, total_nfu, total_optimal);

    /* Determina o algoritmo não-ótimo mais próximo do ótimo */
    int diff_fifo = abs(total_fifo - total_optimal);
    int diff_lru  = abs(total_lru  - total_optimal);
    int diff_nfu  = abs(total_nfu  - total_optimal);

    const char *best      = "FIFO";
    int         best_diff = diff_fifo;
    
    if (diff_lru < best_diff) { best = "LRU"; best_diff = diff_lru; }
    if (diff_nfu < best_diff) { best = "NFU"; }

    log_printf("%s\n", best);
}