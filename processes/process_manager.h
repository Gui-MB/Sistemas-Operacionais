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
    int chance_request_io;     // Chance (%) de solicitar E/S durante a fatia de CPU
    int blocked;               // 1 se bloqueado (na fila ou em uso de um dispositivo), 0 caso contrário
    int requested_device_id;   // Dispositivo alvo da solicitação de E/S atual (-1 se nenhum)
    int device_in_use;         // 1 se já está sendo atendido pelo dispositivo, 0 se apenas na fila de espera
    int io_remaining_time;     // Tempo restante da operação de E/S em andamento
    int io_offset_ticks;       // Instante (dentro da fatia atual) em que a E/S será solicitada (-1 se não solicitará)
    int planned_device_id;     // Dispositivo sorteado para a solicitação da fatia atual
    int ready_wait_time;       // Tempo total acumulado no estado pronto
    int blocked_time;          // Tempo total acumulado no estado bloqueado
    int last_ready_entry_time; // Instante em que o processo entrou no estado pronto pela última vez
    int io_wait_start_time;    // Instante em que o processo entrou em estado bloqueado
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

#endif