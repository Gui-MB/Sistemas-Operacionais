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

#ifdef RR_DEBUG
    printf("[RR_DEBUG] enqueue pid=%d\n",
           processes[process_idx].pid);
#endif
}

static int dequeue() {
    if (is_queue_empty()) {
        return -1;
    }

    int idx = ready_queue[front];
    front = (front + 1) % MAX_QUEUE;

    in_queue[idx] = 0;

#ifdef RR_DEBUG
    printf("[RR_DEBUG] dequeue pid=%d\n",
           processes[idx].pid);
#endif

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
// Seleciona próximo processo
// =========================

int get_next_rr(int current_time) {

#ifdef RR_DEBUG
    printf("\n[RR_DEBUG] get_next_rr time=%d\n",
           current_time);
#endif

    // Adiciona novos processos que chegaram
    update_ready_queue(current_time);

    // Retira da frente da fila
    int idx = dequeue();

    if (idx == -1) {

#ifdef RR_DEBUG
        printf("[RR_DEBUG] fila vazia\n");
#endif

        return -1;
    }

#ifdef RR_DEBUG
    printf("[RR_DEBUG] selected pid=%d\n",
           processes[idx].pid);
#endif

    return idx;
}


// =========================
// Reinsere processo preemptado
// =========================

void rr_requeue(int idx) {

    Process *p = &processes[idx];

    if (!p->is_completed && p->remaining_time > 0) {

        enqueue(idx);

#ifdef RR_DEBUG
        printf("[RR_DEBUG] requeue pid=%d\n",
               p->pid);
#endif
    }
}