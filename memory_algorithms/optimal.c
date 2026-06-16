#include "optimal.h"
#include "../auxiliary_files/processes.h"

int optimal_simulate(int frames, const int *sequence, int seq_len, int print_seq) {
    int memory[frames];
    int size = 0;
    int trocas = 0;

    if (print_seq) log_printf("\n--- Simulação ÓTIMO ---\n");

    for (int i = 0; i < seq_len; i++) {
        int page = sequence[i];
        int found = 0;

        for (int j = 0; j < size; j++) {
            if (memory[j] == page) {
                found = 1;
                break;
            }
        }

        const char *status;
        if (!found) {
            if (size < frames) {
                memory[size++] = page;
                status = "Preenchimento";
            } else {
                int replace_idx = -1;
                int farthest_next_use = -1;

                for (int j = 0; j < frames; j++) {
                    int next_use = -1;
                    for (int k = i + 1; k < seq_len; k++) {
                        if (sequence[k] == memory[j]) {
                            next_use = k;
                            break;
                        }
                    }

                    if (next_use == -1) {
                        replace_idx = j;
                        break;
                    }

                    if (next_use > farthest_next_use) {
                        farthest_next_use = next_use;
                        replace_idx = j;
                    }
                }

                memory[replace_idx] = page;
                trocas++;
                status = "Troca";
            }
        } else {
            status = "Acerto";
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