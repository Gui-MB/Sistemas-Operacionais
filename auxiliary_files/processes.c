#include "processes.h"
#include "devices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Implementação própria e portátil de leitura de linha (equivalente ao getline() do POSIX,
// que não está disponível em todos os compiladores/ambientes, como o MinGW no Windows).
// Lê uma linha inteira (até '\n' ou EOF) para dentro de *lineptr, expandindo o buffer conforme
// necessário. Retorna o número de caracteres lidos, ou -1 em caso de erro/fim de arquivo.
static long portable_getline(char **lineptr, size_t *n, FILE *stream) {
    if (*lineptr == NULL || *n == 0) {
        *n = 256;
        *lineptr = malloc(*n);
        if (!*lineptr) return -1;
    }

    size_t pos = 0;
    for (;;) {
        if (pos + 1 >= *n) {
            size_t new_size = *n * 2;
            char *new_ptr = realloc(*lineptr, new_size);
            if (!new_ptr) return -1;
            *n = new_size;
            *lineptr = new_ptr;
        }

        int c = fgetc(stream);
        if (c == EOF) {
            if (pos == 0) return -1;
            break;
        }

        (*lineptr)[pos++] = (char)c;
        if (c == '\n') break;
    }

    (*lineptr)[pos] = '\0';
    return (long)pos;
}

// Implementação própria e portátil de strdup (também não garantida em todo compilador/ambiente)
static char *portable_strdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *copy = malloc(len);
    if (!copy) return NULL;
    memcpy(copy, src, len);
    return copy;
}

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
    char *copy = portable_strdup(sequence);
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

// Lê o arquivo de entrada e inicializa a lista de processos e de dispositivos
void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_printf("Erro ao abrir o arquivo %s!\n", filename);
        exit(1);
    }

    free_input_data();

    char *line = NULL;
    size_t line_size = 0;

    int num_devices_declared = 0;

    // Primeira linha: configuração geral, agora incluindo numDispositivosES
    if (portable_getline(&line, &line_size, file) != -1) {
        sscanf(line, "%49[^|]|%d|%49[^|]|%d|%d|%d|%d",
               algorithm, &time_slice, memory_policy,
               &memory_size_bytes, &page_size_bytes, &allocation_percent, &num_devices_declared);
        algorithm[strcspn(algorithm, "\r\n")] = 0;
        memory_policy[strcspn(memory_policy, "\r\n")] = 0;
    }

    if (num_devices_declared < 0) num_devices_declared = 0;
    if (num_devices_declared > MAX_DEVICES) num_devices_declared = MAX_DEVICES;
    devices_init(num_devices_declared);

    // Linhas de dispositivos: idDispositivo|numUsosSimultaneos|tempoOperacao (idDispositivo pode ser texto, ex: "device-0")
    for (int d = 0; d < num_devices_declared; d++) {
        if (portable_getline(&line, &line_size, file) == -1) break;

        line[strcspn(line, "\r\n")] = 0;

        char dev_id[MAX_DEVICE_ID_LEN] = "";
        int dev_slots = 1, dev_op_time = 1;
        sscanf(line, "%63[^|]|%d|%d", dev_id, &dev_slots, &dev_op_time);
        if (dev_slots <= 0) dev_slots = 1;
        if (dev_slots > MAX_DEVICE_SLOTS) dev_slots = MAX_DEVICE_SLOTS;
        if (dev_op_time <= 0) dev_op_time = 1;

        devices_set(d, dev_id, dev_slots, dev_op_time);
    }

    // Linhas de processos: tempoCriacao|PID|tempoExec|prioridade|qtdeMemoria|sequenciaPaginas|chanceRequisitarES
    while (portable_getline(&line, &line_size, file) != -1) {
        char *save_ptr = NULL;
        char *creation = strtok_r(line, "|", &save_ptr);
        char *pid = strtok_r(NULL, "|", &save_ptr);
        char *exec_time = strtok_r(NULL, "|", &save_ptr);
        char *priority = strtok_r(NULL, "|", &save_ptr);
        char *memory = strtok_r(NULL, "|", &save_ptr);
        char *tail = strtok_r(NULL, "", &save_ptr); // "sequenciaDePaginas|chanceRequisitarES" (ou só a sequência, no formato antigo)

        if (!creation || !pid || !exec_time || !priority || !memory || !tail) continue;

        // Remove quebras de linha do final antes de procurar o último separador
        tail[strcspn(tail, "\r\n")] = 0;

        // Separa a sequência de páginas da chance de requisitar E/S: ambas ficam após o campo "memory",
        // então isolamos o último campo (chance) pelo último '|' que sobrar na cauda.
        char *last_pipe = strrchr(tail, '|');
        char *sequence;
        char *chance_str = NULL;
        if (last_pipe) {
            *last_pipe = '\0';
            sequence = tail;
            chance_str = last_pipe + 1;
        } else {
            // Compatibilidade com o formato anterior (sem chanceRequisitarES)
            sequence = tail;
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
        process->chance_requisitar_es = chance_str ? (int)strtol(chance_str, NULL, 10) : 0;
        if (process->chance_requisitar_es < 0) process->chance_requisitar_es = 0;
        if (process->chance_requisitar_es > 100) process->chance_requisitar_es = 100;
        
        if (process->virtual_pages <= 0) process->virtual_pages = 1;
        process->frame_limit = (process->virtual_pages * allocation_percent + 99) / 100;
        if (process->frame_limit <= 0) process->frame_limit = 1;
        if (process->frame_limit > process->virtual_pages) process->frame_limit = process->virtual_pages;

        process->page_sequence = calloc((size_t)process->page_sequence_len, sizeof(int));
        if (!process->page_sequence && process->page_sequence_len > 0) {
            fclose(file); free(line); exit(1);
        }

        char *seq_copy = portable_strdup(sequence);
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

        // Estado inicial de E/S (Trabalho OS 3)
        process->state = STATE_READY;
        process->device_index = -1;
        process->waiting_in_queue = 0;
        process->io_will_request = 0;
        process->io_trigger_tick = -1;
        process->io_device_choice = -1;
        process->ready_time_accum = 0;
        process->blocked_time_accum = 0;

        num_processes++;
    }

    free(line);
    fclose(file);
}
