#include "lottery.h"
#include <stdlib.h>
#include <string.h>
#include "../auxiliary_files/processes.h"
#define MAX_PROCESSES 1024   // Número arbitrário para o máximo de processos suportados. Para o contexto do trabalho é suficiente

static int seg_tree[4 * MAX_PROCESSES]; // segment tree para armazenar a soma dos bilhetes (prioridades) em cada segmento
static int leaf_tickets[MAX_PROCESSES]; // bilhetes (prioridades) dos processos nas folhas da segment tree

void build_initial(int current_time) {
    // Função para construir a segment tree inicialmente, atribuindo bilhetes (prioridades) aos processos que já chegaram no tempo inicial

	for (int i = 0; i < num_processes; i++)
        leaf_tickets[i] = 0;
    memset(seg_tree, 0, sizeof(seg_tree));

    for (int i = 0; i < num_processes; i++) {
        if (processes[i].creation_time <= current_time && !processes[i].is_completed)
            update_process(i, processes[i].priority);
    }
}

void activate_arrived(int current_time) {
	// Função para ativar processos que chegaram no tempo atual, atribuindo a eles seus bilhetes (prioridades) na segment tree

    for (int i = 0; i < num_processes; i++) {
        if (processes[i].creation_time <= current_time
            && !processes[i].is_completed) {
            update_process(i, processes[i].priority);
        }
    }
}

void update_process(int proc_idx, int new_tickets) {
	// Função que atualiza a quantidade de bilhetes (soma das prioridades) de um processo e reflete essa mudança na segment tree
	
	int old = leaf_tickets[proc_idx];
    int delta = new_tickets - old;
    if (delta == 0) return;
    leaf_tickets[proc_idx] = new_tickets;

    // Atualiza a segment tree com a nova quantidade de bilhetes do processo
    int node = 1, left = 0, right = num_processes - 1;
    while (1) {
        seg_tree[node] += delta;
        if (left == right)
            break;
        int mid = (left + right) / 2;
        if (proc_idx <= mid) {
            node = node * 2;
            right = mid;
        } else {
            node = node * 2 + 1;
            left = mid + 1;
        }
    }
}

void lottery_process_finished(int proc_idx) {
    update_process(proc_idx, 0);
}

int get_next_lottery(int current_time) {
	// Sorteia um processo proporcional aos seus bilhetes (prioridade).
    
	activate_arrived(current_time);   // ativa processos recém-chegados

    int total_tickets = seg_tree[1];  // raiz contém soma total
    if (total_tickets == 0) return -1;

    int r = rand() % total_tickets;
    int node = 1, left = 0, right = num_processes - 1;
    while (left < right) {
        int mid = (left + right) / 2;
        int left_child = node * 2;
        if (r < seg_tree[left_child]) {
            node = left_child;
            right = mid;
        } else {
            r -= seg_tree[left_child];
            node = left_child + 1;
            left = mid + 1;
        }
    }
    return left;
}