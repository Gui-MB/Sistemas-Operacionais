#include "round_robin.h"
#include "../auxiliary_files/processes.h"
#include <stdio.h>

#define MAX_QUEUE 1000

// Fila FIFO de processos prontos
static int ready_queue[MAX_QUEUE];

static int front = 0;
static int rear = 0;

// Evita inserir o mesmo processo várias vezes
static int in_queue[MAX_QUEUE];

// Marca processos já adicionados após criação
static int already_arrived[MAX_QUEUE];

// Último processo executado (para re-enfileirar após o slice, se ainda estiver pronto)
static int last_executed = -1;


// =========================
// Operações da fila FIFO
// =========================

static int is_queue_empty() {
    return front == rear;
}

static void enqueue(int process_idx) {
    ready_queue[rear] = process_idx;
    rear = (rear + 1) % MAX_QUEUE;

    in_queue[process_idx] = 1;

}

static int dequeue() {
    if (is_queue_empty()) {
        return -1;
    }

    int idx = ready_queue[front];
    front = (front + 1) % MAX_QUEUE;

    in_queue[idx] = 0;

    return idx;
}


// =========================
// Atualiza fila de prontos
// =========================

static void update_ready_queue(int current_time) {

    for (int i = 0; i < num_processes; i++) {

        Process *p = &processes[i];

        // Processo chegou agora
        if (!already_arrived[i]
            && p->creation_time <= current_time
            && !p->is_completed
            && p->remaining_time > 0) {

            enqueue(i);
            already_arrived[i] = 1;
        }
    }
}


// =========================
// Notifica que um processo bloqueado por E/S terminou sua operação e voltou a ficar pronto
// =========================

// Chamada pelo main.c quando um dispositivo libera um processo: o processo volta ao final da fila,
// como esperado no comportamento padrão de alternância circular.
void rr_enqueue_unblocked(int process_idx) {
    if (process_idx < 0 || process_idx >= num_processes) return;
    if (!in_queue[process_idx]) {
        enqueue(process_idx);
    }
}


// =========================
// Seleciona próximo processo
// =========================

int get_next_rr(int current_time) {

    // Adiciona novos processos que chegaram
    update_ready_queue(current_time);

    // Re-enfileira o último processo se ele ainda estiver pronto (não terminou e não foi bloqueado por E/S)
    if (last_executed != -1) {
        Process *last = &processes[last_executed];
        if (!last->is_completed && last->remaining_time > 0 && last->state != STATE_BLOCKED && !in_queue[last_executed]) {
            enqueue(last_executed);
        }
    }

    // Retira da frente da fila
    int idx = dequeue();

    if (idx == -1) {

        return -1;
    }

    last_executed = idx;
    return idx;
}
