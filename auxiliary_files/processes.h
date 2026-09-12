#ifndef PROCESSES_H
#define PROCESSES_H
#define MAX_PROCESSES 1000

// Estados possíveis de um processo em relação à CPU
#define STATE_READY   0
#define STATE_RUNNING 1
#define STATE_BLOCKED 2

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

    // --- Campos de E/S (Trabalho OS 3) ---
    int chance_requisitar_es;     // 0-100: chance de solicitar E/S ao ser escalonado
    int state;                    // STATE_READY, STATE_RUNNING ou STATE_BLOCKED
    int device_index;             // índice do dispositivo em uso/aguardado, -1 se não bloqueado
    int waiting_in_queue;         // 1 se está na fila de espera do dispositivo, 0 se em operação
    int io_will_request;          // 1 se, na fatia atual, o processo vai requisitar E/S
    int io_trigger_tick;          // ciclo (relativo ao início da fatia) em que a E/S será solicitada
    int io_device_choice;         // índice do dispositivo sorteado para a requisição da fatia atual
    int ready_time_accum;         // tempo total acumulado em estado "pronto"
    int blocked_time_accum;       // tempo total acumulado em estado "bloqueado"
} Process;

// Configurações globais lidas do arquivo de entrada

// Array global para armazenar os processos lidos do arquivo de entrada
extern Process processes[MAX_PROCESSES]; 

// Número total de processos lidos do arquivo de entrada
extern int num_processes;

// Time slice fornecido ao escalonador (para algoritmos que utilizam time slice)
extern int time_slice;

// Algoritmo de escalonamento: "alternanciaCircular", "prioridade", "loteria" ou "CFS"
extern char algorithm[50];

// Política de memória: "global" ou "local"
extern char memory_policy[50];

// Tamanho total da memória em bytes (para cálculos de memória)
extern int memory_size_bytes;

// Tamanho da página em bytes (para cálculos de memória)
extern int page_size_bytes;

// Percentual de alocação de frames para cada processo (para memória local)
extern int allocation_percent;

// Fila global para simulação de memória compartilhada
extern int *global_page_sequence;

// Comprimento atual da fila global de acesso à memória
extern int global_sequence_len;

// Função para ler o arquivo de entrada e inicializar a lista de processos
void read_input_file(const char *filename);

// Função para liberar toda memória alocada dinamicamente das entradas e da fila global
void free_input_data(void);

// Função para registrar um acesso de memória à fila global, codificando PID e Página
void record_memory_access(int pid, int page);

// Inclusão do arquivo de logs
#include "logs.h" 

#endif
