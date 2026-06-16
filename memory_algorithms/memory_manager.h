#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "../auxiliary_files/processes.h"

/*
 * Resultado da simulação de substituição de páginas para um processo.
 */
typedef struct {
    int fifo_faults;
    int lru_faults;
    int nfu_faults;
    int optimal_faults;
} PageFaultResult;

/*
 * Executa os quatro algoritmos de substituição de página para um processo
 * e retorna o número de page faults de cada um.
 *
 * p           : ponteiro para o processo a simular
 * page_size   : tamanho da página em bytes (para calcular número de frames)
 * result      : saída com os contadores de page faults
 */
void memory_manager_simulate(const Process *p, PageFaultResult *result);

/*
 * Imprime a linha de saída exigida pelo enunciado:
 * fifo|lru|nfu|optimal|melhor_algoritmo
 *
 * Os totais são a soma de todos os processos simulados.
 */
void memory_manager_print_results(int total_fifo, int total_lru,
                                  int total_nfu, int total_optimal);

#endif /* MEMORY_MANAGER_H */