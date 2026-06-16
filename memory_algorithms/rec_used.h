#ifndef REC_USED_H
#define REC_USED_H

#ifndef MAX_FRAMES
#define MAX_FRAMES 256
#endif

/*
 * Simula o algoritmo LRU (Menos Recentemente Usada) de substituição de páginas.
 *
 * frame_count : número de molduras disponíveis
 * sequence    : sequência de páginas acessadas
 * seq_len     : comprimento da sequência
 *
 * Retorna o total de page faults.
 */
int lru_simulate(int frame_count, const int *sequence, int seq_len);

#endif /* REC_USED_H */