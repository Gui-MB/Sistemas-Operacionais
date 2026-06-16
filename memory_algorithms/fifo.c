#include "fifo.h"
#include <string.h>

/*
 * FIFO – First In, First Out
 *
 * Mantém um array circular que age como fila: a página mais antiga
 * (primeira a entrar) é sempre a primeira a ser substituída.
 *
 * frame_count : número de molduras disponíveis para o processo
 * sequence    : sequência de páginas acessadas
 * seq_len     : comprimento da sequência
 *
 * Retorna o número de page faults ocorridos.
 */
int fifo_simulate(int frame_count, const int *sequence, int seq_len) {
    int frames[MAX_FRAMES];
    int faults = 0;
    int next_replace = 0;   /* índice circular da próxima moldura a substituir */
    int loaded = 0;         /* quantas molduras estão ocupadas */

    memset(frames, -1, sizeof(frames));

    for (int i = 0; i < seq_len; i++) {
        int page = sequence[i];
        int found = 0;

        /* verifica se a página já está em alguma moldura */
        for (int f = 0; f < loaded; f++) {
            if (frames[f] == page) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* page fault: insere na posição circular */
            frames[next_replace] = page;
            next_replace = (next_replace + 1) % frame_count;
            if (loaded < frame_count) {
                loaded++;
            }
            faults++;
        }
    }

    return faults;
}