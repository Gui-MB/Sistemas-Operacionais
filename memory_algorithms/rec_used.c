#include "rec_used.h"
#include <string.h>

/*
 * LRU – Least Recently Used (Menos Recentemente Usada)
 *
 * Para cada page fault, substitui a página cujo último acesso foi
 * o mais distante no passado.  O "tempo do último uso" é rastreado
 * pelo índice da última ocorrência na sequência processada até agora.
 *
 * frame_count : número de molduras disponíveis para o processo
 * sequence    : sequência de páginas acessadas
 * seq_len     : comprimento da sequência
 *
 * Retorna o número de page faults ocorridos.
 */
int lru_simulate(int frame_count, const int *sequence, int seq_len) {
    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES]; /* instante do último acesso para cada moldura */
    int faults = 0;
    int loaded = 0;

    memset(frames,    -1, sizeof(frames));
    memset(last_used,  0, sizeof(last_used));

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
            /* hit: atualiza instante de último acesso */
            last_used[found] = i;
        } else {
            /* page fault */
            faults++;

            if (loaded < frame_count) {
                /* ainda há moldura livre */
                frames[loaded]    = page;
                last_used[loaded] = i;
                loaded++;
            } else {
                /* encontra a moldura menos recentemente usada */
                int lru_frame = 0;
                for (int f = 1; f < frame_count; f++) {
                    if (last_used[f] < last_used[lru_frame]) {
                        lru_frame = f;
                    }
                }
                frames[lru_frame]    = page;
                last_used[lru_frame] = i;
            }
        }
    }

    return faults;
}