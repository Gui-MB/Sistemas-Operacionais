#include "rec_used.h"
#include "../auxiliary_files/processes.h"

int lru_simulate(int frames, const int *sequence, int seq_len, int print_seq) {
    int memory[frames];
    int last_used[frames];
    int size = 0;
    int trocas = 0;

    if (print_seq) log_printf("\n--- Simulação LRU ---\n");

    for (int i = 0; i < seq_len; i++) {
        int page = sequence[i];
        int found = -1;

        for (int j = 0; j < size; j++) {
            if (memory[j] == page) {
                found = j;
                break;
            }
        }

        const char *status;
        if (found != -1) {
            last_used[found] = i; 
            status = "Acerto";
        } else {
            if (size < frames) {
                memory[size] = page;
                last_used[size] = i;
                size++;
                status = "Preenchimento";
            } else {
                int lru_idx = 0;
                for (int j = 1; j < frames; j++) {
                    if (last_used[j] < last_used[lru_idx]) {
                        lru_idx = j;
                    }
                }
                memory[lru_idx] = page;
                last_used[lru_idx] = i;
                trocas++;
                status = "Troca";
            }
        }

        if (print_seq) {
            log_printf("Ref: %2d | Mem: [", page);
            for (int j = 0; j < frames; j++) {
                if (j < size) log_printf("%2d", memory[j]); else log_printf(" -");
                if (j < frames - 1) log_printf(", ");
            }
            log_printf("] | %s\n", status);
        }
    }
    return trocas;
}