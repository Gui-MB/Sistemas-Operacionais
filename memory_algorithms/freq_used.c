#include "freq_used.h"
#include <string.h>

/*
 * NFU – Not Frequently Used (Não Usada Frequentemente)
 *
 * Cada moldura mantém um contador de frequência de acesso.
 * A cada acesso, o contador da página presente é incrementado.
 * Em caso de page fault, substitui a página com menor contador.
 * Em empate, substitui a de menor índice de moldura (FIFO de desempate).
 *
 * Nota: os contadores NÃO são zerados ao substituir a página,
 * mantendo o comportamento clássico do NFU sem aging.
 *
 * frame_count : número de molduras disponíveis para o processo
 * sequence    : sequência de páginas acessadas
 * seq_len     : comprimento da sequência
 *
 * Retorna o número de page faults ocorridos.
 */
int nfu_simulate(int frame_count, const int *sequence, int seq_len) {
    int frames[MAX_FRAMES];
    int freq[MAX_FRAMES];   /* contador de acessos por moldura */
    int faults = 0;
    int loaded = 0;

    memset(frames, -1, sizeof(frames));
    memset(freq,    0, sizeof(freq));

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
            /* hit: incrementa frequência */
            freq[found]++;
        } else {
            /* page fault */
            faults++;

            if (loaded < frame_count) {
                /* moldura livre */
                frames[loaded] = page;
                freq[loaded]   = 1;
                loaded++;
            } else {
                /* substitui a de menor frequência */
                int victim = 0;
                for (int f = 1; f < frame_count; f++) {
                    if (freq[f] < freq[victim]) {
                        victim = f;
                    }
                }
                frames[victim] = page;
                freq[victim]   = 1; /* reinicia contador para a nova página */
            }
        }
    }

    return faults;
}