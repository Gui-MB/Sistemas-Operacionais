#include "io_manager.h"

#include <stdlib.h>
#include <string.h>

#include "../logs/log_manager.h"
#include "../processes/process_manager.h"

Device devices[MAX_DEVICES];
int num_devices = 0;

// Reinicia o estado do gerenciador de E/S (chamado antes de ler uma nova entrada)
void io_manager_reset(void) {
    num_devices = 0;
    memset(devices, 0, sizeof(devices));
}

// Registra um dispositivo lido do arquivo de entrada
void io_manager_add_device(const char *name, int max_concurrent, int operation_time) {
    if (num_devices >= MAX_DEVICES) return;

    Device *d = &devices[num_devices];
    strncpy(d->name, name, sizeof(d->name) - 1);
    d->name[sizeof(d->name) - 1] = '\0';
    d->max_concurrent = max_concurrent;
    d->operation_time = operation_time;
    d->current_users = 0;
    d->waiting_head = 0;
    d->waiting_tail = 0;
    d->waiting_count = 0;

    num_devices++;
}

// Decide, via probabilidade (chanceRequisitarES), se o processo solicitará E/S nesta fatia de CPU
int io_manager_should_request(const Process *p) {
    if (num_devices <= 0) return 0;
    return (rand() % 100) < p->chance_request_io;
}

// Sorteia, dentre os dispositivos existentes, qual será solicitado
int io_manager_pick_device(void) {
    if (num_devices <= 0) return -1;
    return rand() % num_devices;
}

// Coloca o processo na fila de espera de um dispositivo
static void enqueue_waiting(Device *d, int proc_idx) {
    d->waiting_queue[d->waiting_tail] = proc_idx;
    d->waiting_tail = (d->waiting_tail + 1) % MAX_PROCESSES;
    d->waiting_count++;
}

// Retira o próximo processo da fila de espera de um dispositivo
static int dequeue_waiting(Device *d) {
    int proc_idx = d->waiting_queue[d->waiting_head];
    d->waiting_head = (d->waiting_head + 1) % MAX_PROCESSES;
    d->waiting_count--;
    return proc_idx;
}

// Solicita o uso de um dispositivo para o processo indicado, bloqueando-o
void io_manager_request(int proc_idx, int device_id, int current_time) {
    if (device_id < 0 || device_id >= num_devices) return;

    Device *d = &devices[device_id];
    Process *p = &processes[proc_idx];

    p->blocked = 1;
    p->requested_device_id = device_id;
    p->io_wait_start_time = current_time;

    if (d->current_users < d->max_concurrent) {
        p->device_in_use = 1;
        p->io_remaining_time = d->operation_time;
        d->active_procs[d->current_users++] = proc_idx;
    } else {
        p->device_in_use = 0;
        enqueue_waiting(d, proc_idx);
    }
}

// Avança em um tick o estado de todos os dispositivos (operações em andamento e filas de espera)
void io_manager_tick(int current_time) {
    for (int di = 0; di < num_devices; di++) {
        Device *d = &devices[di];

        // Avança as operações em andamento e libera os processos que terminaram
        int i = 0;
        while (i < d->current_users) {
            int idx = d->active_procs[i];
            Process *p = &processes[idx];
            p->io_remaining_time--;

            if (p->io_remaining_time <= 0) {
                p->blocked = 0;
                p->device_in_use = 0;
                p->requested_device_id = -1;
                p->blocked_time += current_time - p->io_wait_start_time;
                p->last_ready_entry_time = current_time;

                // Remove o processo da lista de ativos (troca com o último elemento)
                d->active_procs[i] = d->active_procs[d->current_users - 1];
                d->current_users--;
            } else {
                i++;
            }
        }

        // Preenche vagas livres com processos que aguardam na fila
        while (d->current_users < d->max_concurrent && d->waiting_count > 0) {
            int idx = dequeue_waiting(d);
            Process *p = &processes[idx];
            p->device_in_use = 1;
            p->io_remaining_time = d->operation_time;
            d->active_procs[d->current_users++] = idx;
        }
    }
}

// Imprime o estado atual de todos os dispositivos (em uso e na fila de espera)
void io_manager_print_state(void) {
    log_printf("Dispositivos de E/S:\n");
    for (int di = 0; di < num_devices; di++) {
        Device *d = &devices[di];
        log_printf("  %s (em uso: %d/%d | aguardando: %d)\n", d->name, d->current_users, d->max_concurrent, d->waiting_count);

        if (d->current_users > 0) {
            log_printf("    Em uso: ");
            for (int i = 0; i < d->current_users; i++) {
                log_printf("pid=%d ", processes[d->active_procs[i]].pid);
            }
            log_printf("\n");
        }

        if (d->waiting_count > 0) {
            log_printf("    Fila de espera: ");
            for (int i = 0; i < d->waiting_count; i++) {
                int idx = d->waiting_queue[(d->waiting_head + i) % MAX_PROCESSES];
                log_printf("pid=%d ", processes[idx].pid);
            }
            log_printf("\n");
        }
    }
}
