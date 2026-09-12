#include "devices.h"
#include "logs.h"
#include <stddef.h>
#include <string.h>

Device devices[MAX_DEVICES];
int num_devices = 0;

int finished_pids[MAX_FINISHED_PER_TICK];
int finished_count = 0;

void devices_init(int count) {
    num_devices = count;
    for (int i = 0; i < count; i++) {
        devices[i].queue_len = 0;
        devices[i].id[0] = '\0';
        for (int s = 0; s < MAX_DEVICE_SLOTS; s++) {
            devices[i].slot_pid[s] = -1;
            devices[i].slot_remaining[s] = 0;
        }
    }
}

void devices_set(int index, const char *id, int num_simultaneous, int operation_time) {
    if (index < 0 || index >= num_devices) return;
    strncpy(devices[index].id, id, MAX_DEVICE_ID_LEN - 1);
    devices[index].id[MAX_DEVICE_ID_LEN - 1] = '\0';
    devices[index].num_simultaneous = num_simultaneous;
    devices[index].operation_time = operation_time;
}

int device_find_by_id(const char *id) {
    for (int i = 0; i < num_devices; i++) {
        if (strcmp(devices[i].id, id) == 0) return i;
    }
    return -1;
}

// Tenta ocupar um slot livre do dispositivo; se não houver, o processo entra na fila de espera (FIFO)
int device_request(int device_idx, int pid) {
    Device *d = &devices[device_idx];

    for (int s = 0; s < d->num_simultaneous && s < MAX_DEVICE_SLOTS; s++) {
        if (d->slot_pid[s] == -1) {
            d->slot_pid[s] = pid;
            d->slot_remaining[s] = d->operation_time;
            return 1;
        }
    }

    if (d->queue_len < MAX_DEVICE_QUEUE_LEN) {
        d->queue[d->queue_len++] = pid;
    }
    return 0;
}

// Avança 1 ciclo de tempo em todos os dispositivos e promove o próximo da fila quando um slot libera
void device_tick(void) {
    finished_count = 0;

    for (int i = 0; i < num_devices; i++) {
        Device *d = &devices[i];
        for (int s = 0; s < d->num_simultaneous && s < MAX_DEVICE_SLOTS; s++) {
            if (d->slot_pid[s] == -1) continue;

            d->slot_remaining[s]--;
            if (d->slot_remaining[s] <= 0) {
                if (finished_count < MAX_FINISHED_PER_TICK) {
                    finished_pids[finished_count++] = d->slot_pid[s];
                }
                d->slot_pid[s] = -1;

                // Promove o próximo processo da fila de espera para este slot recém-liberado
                if (d->queue_len > 0) {
                    int next_pid = d->queue[0];
                    for (int q = 1; q < d->queue_len; q++) {
                        d->queue[q - 1] = d->queue[q];
                    }
                    d->queue_len--;

                    d->slot_pid[s] = next_pid;
                    d->slot_remaining[s] = d->operation_time;
                }
            }
        }
    }
}

int device_status_of_pid(int pid, int *is_waiting_out) {
    for (int i = 0; i < num_devices; i++) {
        Device *d = &devices[i];
        for (int s = 0; s < d->num_simultaneous && s < MAX_DEVICE_SLOTS; s++) {
            if (d->slot_pid[s] == pid) {
                *is_waiting_out = 0;
                return i;
            }
        }
        for (int q = 0; q < d->queue_len; q++) {
            if (d->queue[q] == pid) {
                *is_waiting_out = 1;
                return i;
            }
        }
    }
    return -1;
}

void devices_print_state(void) {
    log_printf("  Dispositivos:\n");
    if (num_devices == 0) {
        log_printf("    (nenhum dispositivo configurado)\n");
        return;
    }

    for (int i = 0; i < num_devices; i++) {
        Device *d = &devices[i];
        log_printf("    Dispositivo %s (slots=%d, tempo_operacao=%d):", d->id, d->num_simultaneous, d->operation_time);

        int any_busy = 0;
        for (int s = 0; s < d->num_simultaneous && s < MAX_DEVICE_SLOTS; s++) {
            if (d->slot_pid[s] != -1) {
                log_printf(" [uso: PID %d, restante=%d]", d->slot_pid[s], d->slot_remaining[s]);
                any_busy = 1;
            }
        }
        if (!any_busy) log_printf(" livre");

        if (d->queue_len > 0) {
            log_printf(" | fila:");
            for (int q = 0; q < d->queue_len; q++) {
                log_printf(" PID %d", d->queue[q]);
            }
        }
        log_printf("\n");
    }
}
