#ifndef OPTIMAL_H
#define OPTIMAL_H

#ifndef MAX_FRAMES
#define MAX_FRAMES 256
#endif

/*
 * Simula o algoritmo Ótimo (Belady) de substituição de páginas.
 *
 * frame_count : número de molduras disponíveis
 * sequence    : sequência completa de páginas acessadas (futuro conhecido)
 * seq_len     : comprimento da sequência
 *
 * Retorna o total de page faults.
 */
int optimal_simulate(int frame_count, const int *sequence, int seq_len);

#endif /* OPTIMAL_H */