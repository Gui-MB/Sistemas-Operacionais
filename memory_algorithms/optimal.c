#include "optimal.h"
#include <string.h>
#include <limits.h>

/*
 * Algoritmo Ótimo (Belady / OPT)
 *
 * Em caso de page fault, substitui a página que será usada mais
 * tarde no futuro (ou nunca mais).  Como o futuro é conhecido,
 * este é o algoritmo de referência com o menor número possível
 * de page faults.
 *
 * frame_count : número de molduras disponíveis para o processo
 * sequence    : sequência completa de páginas acessadas
 * seq_len     : comprimento da sequência
 *
 * Retorna o número de page faults ocorridos.
 */
int optimal_simulate(int frame_count, const int *sequence, int seq_len) {
    int frames[MAX_FRAMES];
    int faults = 0;
    int loaded = 0;

    memset(frames, -1, sizeof(frames));

    for (int i = 0; i < seq_len; i++) {
        int page  = sequence[i];
        int found = -1;

        /* verifica se a página já está em alguma moldura */
        for (int f = 0; f < loaded; f++) {
            if (frames[f] == page) {
                found = f;
                break;
            }
        }

        if (found != -1) {
            /* hit: nada a fazer */
            continue;
        }

        /* page fault */
        faults++;

        if (loaded < frame_count) {
            /* moldura livre */
            frames[loaded++] = page;
            continue;
        }

        /*
         * Escolhe a vítima: a página que será referenciada mais
         * distante no futuro (ou nunca mais referenciada).
         */
        int victim     = 0;
        int farthest   = -1;

        for (int f = 0; f < frame_count; f++) {
            /* procura a próxima ocorrência de frames[f] após posição i */
            int next_use = INT_MAX; /* "nunca mais" */
            for (int j = i + 1; j < seq_len; j++) {
                if (sequence[j] == frames[f]) {
                    next_use = j;
                    break;
                }
            }

            if (next_use > farthest) {
                farthest = next_use;
                victim   = f;
            }
        }

        frames[victim] = page;
    }

    return faults;
}