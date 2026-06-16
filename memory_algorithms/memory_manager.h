#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "../auxiliary_files/processes.h"
typedef struct {
    int fifo_faults;
    int lru_faults;
    int nfu_faults;
    int optimal_faults;
} PageFaultResult;

// Executa os quatro algoritmos de substituição de memória e armazena o número de trocas de página(page fault)
void memory_manager_simulate(const Process *p, PageFaultResult *result);

#endif