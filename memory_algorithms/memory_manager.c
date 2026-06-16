#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "memory_manager.h"
#include "fifo.h"
#include "rec_used.h"
#include "freq_used.h"
#include "optimal.h"

void memory_manager_simulate(const Process *p, PageFaultResult *result) {
    int frames = p->frame_limit;
    int seq_len = p->page_sequence_len;
    const int *seq = p->page_sequence;

    if (frames <= 0 || seq_len <= 0 || seq == NULL) {
        result->fifo_faults = 0;
        result->lru_faults = 0;
        result->nfu_faults = 0;
        result->optimal_faults = 0;
        return;
    }

    result->fifo_faults = fifo_simulate(frames, seq, seq_len);
    result->lru_faults = lru_simulate(frames, seq, seq_len);
    result->nfu_faults = nfu_simulate(frames, seq, seq_len);
    result->optimal_faults = optimal_simulate(frames, seq, seq_len);
}