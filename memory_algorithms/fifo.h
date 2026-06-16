#ifndef FIFO_H
#define FIFO_H

#define MAX_FRAMES 256

/*
 * Simula o algoritmo FIFO de substituição de páginas.
 *
 * frame_count : número de molduras disponíveis
 * sequence    : sequência de páginas acessadas
 * seq_len     : comprimento da sequência
 *
 * Retorna o total de page faults.
 */
int fifo_simulate(int frame_count, const int *sequence, int seq_len);

#endif /* FIFO_H */