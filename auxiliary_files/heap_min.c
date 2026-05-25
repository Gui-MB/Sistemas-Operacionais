#include "heap_min.h"
#include <stddef.h>

// Vetor que representa o heap mínimo e seu estado atual
static Process *heap[MAX_PROCESSES];
static int heap_size = 0;
static int heap_initialized = 0;

// Compara dois processos: menor prioridade vence. Em empate, menor PID vence
static int is_less(const Process *a, const Process *b) {
    if (a->priority != b->priority) {
        return a->priority < b->priority;
    }
    return a->pid < b->pid;
}

// Inicializa o heap mínimo
void heap_min_init(void) {
    heap_size = 0;
    heap_initialized = 1;
}

// Insere um processo no heap mínimo
void heap_min_insert(Process *p) {
    if (!heap_initialized) {
        heap_min_init();
    }
    if (heap_size >= MAX_PROCESSES) {
        return;
    }

    int i = heap_size;
    heap[heap_size++] = p;

    while (i > 0) {
        int parent = (i - 1) / 2;
        if (!is_less(heap[i], heap[parent])) {
            break;
        }
        Process *tmp = heap[i];
        heap[i] = heap[parent];
        heap[parent] = tmp;
        i = parent;
    }
}

// Remove e retorna o processo com menor prioridade do heap
Process *heap_min_pop(void) {
    if (!heap_initialized || heap_size == 0) {
        return NULL;
    }

    Process *min = heap[0];
    heap_size--;

    if (heap_size == 0) {
        return min;
    }

    heap[0] = heap[heap_size];

    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < heap_size && is_less(heap[left], heap[smallest])) {
            smallest = left;
        }
        if (right < heap_size && is_less(heap[right], heap[smallest])) {
            smallest = right;
        }
        if (smallest == i) {
            break;
        }

        Process *tmp = heap[i];
        heap[i] = heap[smallest];
        heap[smallest] = tmp;
        i = smallest;
    }

    return min;
}
