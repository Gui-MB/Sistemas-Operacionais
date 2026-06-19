#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_manager.h"
#include "fifo.h"
#include "rec_used.h"
#include "freq_used.h"
#include "optimal.h"
#include "../auxiliary_files/processes.h"

void memory_manager_simulate(const Process *p, PageFaultResult *result) {
    // O gerenciador de memória recebe um processo já preparado e decide quantas molduras ele pode usar.
    // Em seguida, executa os algoritmos FIFO, LRU, NFU e ÓTIMO sobre a mesma sequência de acessos.
    int frames = p->frame_limit;
    int seq_len = p->page_sequence_len;
    const int *seq = p->page_sequence;

    // Processos sem memória configurada ou sem sequência válida não geram falhas de página.
    if (frames <= 0 || seq_len <= 0 || seq == NULL) {
        result->fifo_faults = 0; result->lru_faults = 0;
        result->nfu_faults = 0; result->optimal_faults = 0;
        return;
    }

    // No modo local, o log mostra a análise isolada de cada processo.
    if (log_cfg.memory_steps) {
        log_printf("\n----------------------------------------------------------------------\n");
        log_printf("Estado de Memória para o processo PID: %d (Frames: %d)\n", p->pid, frames);
        log_printf("----------------------------------------------------------------------");
    }

    // O último parâmetro (0) indica modo LOCAL, já que a simulação aqui é por processo.
    result->fifo_faults = fifo_simulate(frames, seq, seq_len, 0, NULL);
    result->lru_faults = lru_simulate(frames, seq, seq_len, 0, NULL);
    result->nfu_faults = nfu_simulate(frames, seq, seq_len, 0, NULL);
    result->optimal_faults = optimal_simulate(frames, seq, seq_len, 0, NULL);
}