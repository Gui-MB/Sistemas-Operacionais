#include "rec_used.h"
#include "../auxiliary_files/processes.h"

int lru_simulate(int frames, const int *sequence, int seq_len, int is_global) {
    // A LRU substitui a página menos recentemente usada.
    // Para isso, armazenamos o instante do último acesso de cada frame.
    int memory[frames], last_used[frames], size = 0, trocas = 0;

    if (log_cfg.memory_steps) log_printf("\n--- Simulação LRU ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Procura a página na memória para distinguir acerto de falta.
        int page = sequence[i], found = -1;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = j; break; } }

        const char *status;
        // Em caso de acerto, atualizamos o instante do último uso.
        if (found != -1) { last_used[found] = i; status = "Acerto"; }
        else {
            // Ainda existe capacidade livre, então apenas alocamos a página.
            if (size < frames) { memory[size] = page; last_used[size] = i; size++; status = "Preenchimento"; } 
            else {
                // Escolhe o frame com o menor timestamp de uso recente.
                int lru_idx = 0;
                for (int j = 1; j < frames; j++) { if (last_used[j] < last_used[lru_idx]) lru_idx = j; }
                memory[lru_idx] = page; last_used[lru_idx] = i; trocas++; status = "Troca";
            }
        }

        // O log mostra a memória após cada acesso para facilitar a comparação entre algoritmos.
        if (log_cfg.memory_steps) {
            if (is_global) log_printf("Ref: P%d-p%02d | Mem: [", page >> 16, page & 0xFFFF);
            else log_printf("Ref: %2d | Mem: [", page);

            for (int j = 0; j < frames; j++) {
                if (j < size) {
                    if (is_global) log_printf("P%d:p%02d", memory[j] >> 16, memory[j] & 0xFFFF);
                    else log_printf("%2d", memory[j]);
                } else { if (is_global) log_printf("  --  "); else log_printf(" -"); }
                if (j < frames - 1) log_printf(", ");
            }
            log_printf("] | %s\n", status);
        }
    }

    // Apenas substituições contam como trocas de página.
    return trocas;
}