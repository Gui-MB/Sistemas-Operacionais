#include "processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Process processes[MAX_PROCESSES];
int num_processes = 0;
int time_slice = 0;
char algorithm[50];
char memory_policy[50];
int memory_size_bytes = 0;
int page_size_bytes = 0;
int allocation_percent = 0;

int *global_page_sequence = NULL;
int global_sequence_len = 0;
int global_sequence_capacity = 0;

// Adiciona o acesso de memória à fila global, codificando PID e Página
void record_memory_access(int pid, int page) {
    if (global_sequence_len >= global_sequence_capacity) {
        global_sequence_capacity = global_sequence_capacity == 0 ? 1024 : global_sequence_capacity * 2;
        global_page_sequence = realloc(global_page_sequence, global_sequence_capacity * sizeof(int));
    }
    global_page_sequence[global_sequence_len++] = (pid << 16) | (page & 0xFFFF);
}

// Conta o número de páginas na sequência de acesso de um processo
static int count_pages_in_sequence(const char *sequence) {
    int count = 0;
    char *copy = strdup(sequence);
    if (!copy) return 0;

    char *save_ptr = NULL;
    for (char *token = strtok_r(copy, " \t\r\n", &save_ptr); token != NULL; token = strtok_r(NULL, " \t\r\n", &save_ptr)) {
        count++;
    }

    free(copy);
    return count;
}

// Libera toda memória alocada dinamicamente das entradas e da fila global
void free_input_data(void) {
    for (int i = 0; i < num_processes; i++) {
        free(processes[i].page_sequence);
        processes[i].page_sequence = NULL;
        processes[i].page_sequence_len = 0;
    }
    num_processes = 0;
    if (global_page_sequence) {
        free(global_page_sequence);
        global_page_sequence = NULL;
    }
    global_sequence_len = 0;
    global_sequence_capacity = 0;
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

    if (getline(&line, &line_size, file) != -1) {
        sscanf(line, "%49[^|]|%d|%49[^|]|%d|%d|%d",
               algorithm, &time_slice, memory_policy,
               &memory_size_bytes, &page_size_bytes, &allocation_percent);
        algorithm[strcspn(algorithm, "\r\n")] = 0;
        memory_policy[strcspn(memory_policy, "\r\n")] = 0;
    }

    while (getline(&line, &line_size, file) != -1) {
        char *save_ptr = NULL;
        char *creation = strtok_r(line, "|", &save_ptr);
        char *pid = strtok_r(NULL, "|", &save_ptr);
        char *exec_time = strtok_r(NULL, "|", &save_ptr);
        char *priority = strtok_r(NULL, "|", &save_ptr);
        char *memory = strtok_r(NULL, "|", &save_ptr);
        char *sequence = strtok_r(NULL, "", &save_ptr);

        if (!creation || !pid || !exec_time || !priority || !memory || !sequence) continue;

        Process *process = &processes[num_processes];
        memset(process, 0, sizeof(*process));

        process->creation_time = (int)strtol(creation, NULL, 10);
        process->pid = (int)strtol(pid, NULL, 10);
        process->exec_time = (int)strtol(exec_time, NULL, 10);
        process->priority = (int)strtol(priority, NULL, 10);
        process->memory_bytes = (int)strtol(memory, NULL, 10);
        process->page_sequence_len = count_pages_in_sequence(sequence);
        process->virtual_pages = (process->memory_bytes + page_size_bytes - 1) / page_size_bytes;
        
        if (process->virtual_pages <= 0) process->virtual_pages = 1;
        process->frame_limit = (process->virtual_pages * allocation_percent + 99) / 100;
        if (process->frame_limit <= 0) process->frame_limit = 1;
        if (process->frame_limit > process->virtual_pages) process->frame_limit = process->virtual_pages;

        process->page_sequence = calloc((size_t)process->page_sequence_len, sizeof(int));
        if (!process->page_sequence && process->page_sequence_len > 0) {
            fclose(file); free(line); exit(1);
        }

        char *seq_copy = strdup(sequence);
        int page_idx = 0;
        char *seq_save_ptr = NULL;
        for (char *token = strtok_r(seq_copy, " \t\r\n", &seq_save_ptr); token != NULL; token = strtok_r(NULL, " \t\r\n", &seq_save_ptr)) {
            if (page_idx < process->page_sequence_len) {
                process->page_sequence[page_idx++] = (int)strtol(token, NULL, 10);
            }
        }
        free(seq_copy);

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