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
    int frames = p->frame_limit;
    int seq_len = p->page_sequence_len;
    const int *seq = p->page_sequence;

    if (frames <= 0 || seq_len <= 0 || seq == NULL) {
        result->fifo_faults = 0; result->lru_faults = 0;
        result->nfu_faults = 0; result->optimal_faults = 0;
        return;
    }

    if (log_cfg.memory_steps) {
        log_printf("\n----------------------------------------------------------------------\n");
        log_printf("Estado de Memória para o processo PID: %d (Frames: %d)\n", p->pid, frames);
        log_printf("----------------------------------------------------------------------");
    }

    // O último parâmetro (0) indica modo LOCAL (para exibição de logs)
    result->fifo_faults = fifo_simulate(frames, seq, seq_len, 0);
    result->lru_faults = lru_simulate(frames, seq, seq_len, 0);
    result->nfu_faults = nfu_simulate(frames, seq, seq_len, 0);
    result->optimal_faults = optimal_simulate(frames, seq, seq_len, 0);
}