#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "../auxiliary_files/processes.h"

#define MAX_DEVICES 64

// Estrutura para armazenar as informações de cada dispositivo de E/S
typedef struct {
    char name[50];                     // Identificador do dispositivo lido da entrada (ex: "device-0")
    int max_concurrent;                // Quantidade de processos que podem usar o dispositivo simultaneamente
    int operation_time;                // Tempo que o dispositivo demora para executar uma operação
    int active_procs[MAX_PROCESSES];   // Índices dos processos atualmente em uso do dispositivo
    int current_users;                 // Quantidade de processos em uso no momento
    int waiting_queue[MAX_PROCESSES];  // Fila de espera (índices de processos), como buffer circular
    int waiting_head;
    int waiting_tail;
    int waiting_count;
} Device;

// Array global com os dispositivos de E/S lidos do arquivo de entrada
extern Device devices[MAX_DEVICES];

// Número total de dispositivos de E/S lidos do arquivo de entrada
extern int num_devices;

// Reinicia o estado do gerenciador de E/S (chamado antes de ler uma nova entrada)
void io_manager_reset(void);

// Registra um dispositivo lido do arquivo de entrada
void io_manager_add_device(const char *name, int max_concurrent, int operation_time);

// Decide, via probabilidade (chanceRequisitarES), se o processo solicitará E/S nesta fatia de CPU
int io_manager_should_request(const Process *p);

// Sorteia, dentre os dispositivos existentes, qual será solicitado
int io_manager_pick_device(void);

// Solicita o uso de um dispositivo para o processo indicado, bloqueando-o
void io_manager_request(int proc_idx, int device_id, int current_time);

// Avança em um tick o estado de todos os dispositivos (operações em andamento e filas de espera)
void io_manager_tick(int current_time);

// Imprime o estado atual de todos os dispositivos (em uso e na fila de espera)
void io_manager_print_state(void);

#endif
