#include "scheduler_manager.h"

#include <string.h>

#include "../auxiliary_files/processes.h"
#include "cfs.h"
#include "lottery.h"
#include "priority.h"
#include "round_robin.h"

static Algorithm current_algorithm = ALG_UNKNOWN;

// Função para mapear a string do algoritmo para o enum correspondente
static Algorithm parse_algorithm(void) {
    if (strcmp(algorithm, "alternanciaCircular") == 0 || strcmp(algorithm, "alternancia") == 0) {
        return ALG_RR;
    } else if (strcmp(algorithm, "prioridade") == 0) {
        return ALG_PRIORITY;
    } else if (strcmp(algorithm, "loteria") == 0) {
        return ALG_LOTTERY;
    } else if (strcmp(algorithm, "CFS") == 0) {
        return ALG_CFS;
    }
    return ALG_UNKNOWN;
}

// Inicializa o gerenciador de escalonamento, definindo o algoritmo a ser utilizado
void scheduler_manager_init(void) {
    current_algorithm = parse_algorithm();
}

// Retorna o algoritmo de escalonamento atualmente configurado
Algorithm scheduler_manager_get_algorithm(void) {
    return current_algorithm;
}

// Seleciona o próximo processo a ser executado com base no algoritmo configurado
int scheduler_manager_select_next(int current_time, int *had_error) {
    switch (current_algorithm) {
        case ALG_RR:
            return get_next_rr(current_time);
        case ALG_PRIORITY:
            return get_next_priority(current_time);
        case ALG_LOTTERY:
            return get_next_lottery(current_time);
        case ALG_CFS:
            return get_next_cfs(current_time);
        case ALG_UNKNOWN:
        default:
            log_printf("Erro: Algoritmo desconhecido (%s)\n", algorithm);
            *had_error = 1;
            return -1;
    }
}
