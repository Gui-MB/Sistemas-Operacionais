#ifndef DEVICES_H
#define DEVICES_H

#define MAX_DEVICES 50
#define MAX_DEVICE_SLOTS 100
#define MAX_DEVICE_QUEUE_LEN 1000
#define MAX_FINISHED_PER_TICK 1000

#define MAX_DEVICE_ID_LEN 64

// Representa um dispositivo de E/S com múltiplos slots de uso simultâneo e uma fila de espera
typedef struct {
    char id[MAX_DEVICE_ID_LEN];
    int num_simultaneous;
    int operation_time;
    int slot_pid[MAX_DEVICE_SLOTS];       // PID usando cada slot, -1 se livre
    int slot_remaining[MAX_DEVICE_SLOTS]; // tempo restante da operação em cada slot ocupado
    int queue[MAX_DEVICE_QUEUE_LEN];      // fila FIFO de PIDs aguardando o dispositivo
    int queue_len;
} Device;

extern Device devices[MAX_DEVICES];
extern int num_devices;

// PIDs que concluíram uma operação de E/S no último device_tick()
extern int finished_pids[MAX_FINISHED_PER_TICK];
extern int finished_count;

// Inicializa a lista de dispositivos com `count` dispositivos vazios
void devices_init(int count);

// Configura o dispositivo de índice `index` com seus dados lidos da entrada.
// `id` é uma string (ex.: "device-0", "1", etc.), copiada para o dispositivo.
void devices_set(int index, const char *id, int num_simultaneous, int operation_time);

// Retorna o índice do dispositivo com o id informado, ou -1 se não existir
int device_find_by_id(const char *id);

// Solicita o uso do dispositivo `device_idx` pelo processo `pid`.
// Retorna 1 se a operação começou imediatamente (havia slot livre), 0 se o processo entrou na fila de espera.
int device_request(int device_idx, int pid);

// Avança 1 ciclo de tempo em todos os dispositivos: decrementa o tempo restante de cada operação em
// andamento, libera quem terminou (preenchendo finished_pids/finished_count) e promove o próximo da fila.
void device_tick(void);

// Localiza um processo bloqueado (usando ou aguardando) em algum dispositivo.
// Retorna o índice do dispositivo, ou -1 se o pid não estiver em nenhum dispositivo.
// is_waiting_out recebe 1 se está na fila de espera, 0 se está efetivamente em operação.
int device_status_of_pid(int pid, int *is_waiting_out);

// Imprime, via log_printf, o estado de todos os dispositivos (quem usa, quem espera)
void devices_print_state(void);

#endif
