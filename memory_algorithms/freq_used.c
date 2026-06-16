#include "freq_used.h"
#include "../auxiliary_files/processes.h"

int nfu_simulate(int frames, const int *sequence, int seq_len, int print_seq) {
    int memory[frames];
    int freq[frames];
    int size = 0;
    int trocas = 0;

    if (print_seq) log_printf("\n--- Simulação NFU ---\n");

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
            freq[found]++; 
            status = "Acerto";
        } else {
            if (size < frames) {
                memory[size] = page;
                freq[size] = 1; 
                size++;
                status = "Preenchimento";
            } else {
                int nfu_idx = 0;
                for (int j = 1; j < frames; j++) {
                    if (freq[j] < freq[nfu_idx]) {
                        nfu_idx = j;
                    } else if (freq[j] == freq[nfu_idx]) {
                        if (memory[j] < memory[nfu_idx]) {
                            nfu_idx = j;
                        }
                    }
                }
                memory[nfu_idx] = page;
                freq[nfu_idx] = 1; 
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