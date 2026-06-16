#include "memory_manager.h"
#include "fifo.h"
#include "rec_used.h"
#include "freq_used.h"
#include "optimal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Executa os quatro algoritmos de substituição de página para um processo.
 *
 * O número de molduras (frames) disponíveis para o processo é dado por
 * process->frame_limit, calculado em processes.c como:
 *
 *   virtual_pages = ceil(memory_bytes / page_size_bytes)
 *   frame_limit   = ceil(virtual_pages * allocation_percent / 100)
 *
 * Isso já foi resolvido durante a leitura do arquivo de entrada.
 */
void memory_manager_simulate(const Process *p, PageFaultResult *result) {
    int frames   = p->frame_limit;
    int seq_len  = p->page_sequence_len;
    const int *seq = p->page_sequence;

    if (frames <= 0 || seq_len <= 0 || seq == NULL) {
        result->fifo_faults    = 0;
        result->lru_faults     = 0;
        result->nfu_faults     = 0;
        result->optimal_faults = 0;
        return;
    }

    result->fifo_faults    = fifo_simulate(frames, seq, seq_len);
    result->lru_faults     = lru_simulate(frames, seq, seq_len);
    result->nfu_faults     = nfu_simulate(frames, seq, seq_len);
    result->optimal_faults = optimal_simulate(frames, seq, seq_len);
}

/*
 * Imprime a linha de saída exigida pelo enunciado:
 *
 *   fifo|lru|nfu|optimal|nome_do_melhor
 *
 * "Melhor" é o algoritmo não-ótimo cuja contagem de faults
 * é mais próxima (diferença absoluta menor) do algoritmo ótimo.
 * Em empate, a ordem de preferência é: FIFO → LRU → NFU.
 */
void memory_manager_print_results(int total_fifo, int total_lru,
                                  int total_nfu, int total_optimal) {
    int diff_fifo = abs(total_fifo - total_optimal);
    int diff_lru  = abs(total_lru  - total_optimal);
    int diff_nfu  = abs(total_nfu  - total_optimal);

    const char *best = "FIFO";
    int best_diff    = diff_fifo;

    if (diff_lru < best_diff) {
        best      = "LRU";
        best_diff = diff_lru;
    }
    if (diff_nfu < best_diff) {
        best = "NFU";
    }

    printf("%d|%d|%d|%d|%s\n",
           total_fifo, total_lru, total_nfu, total_optimal, best);
}