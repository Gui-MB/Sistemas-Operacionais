#include "freq_used.h"
#include "../auxiliary_files/processes.h"

int nfu_simulate(int frames, const int *sequence, int seq_len, int is_global) {
    // O NFU (Not Frequently Used) mantém um contador de uso por frame.
    // Na falta de página, sai a menos referenciada; em empate, usamos o menor ID.
    int memory[frames], freq[frames], size = 0, trocas = 0;

    if (log_cfg.memory_steps) log_printf("\n--- Simulação NFU ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Primeiro verificamos se a página já está carregada.
        int page = sequence[i], found = -1;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = j; break; } }

        const char *status;
        // Em acerto, apenas incrementamos a frequência observada daquele frame.
        if (found != -1) { freq[found]++; status = "Acerto"; }
        else {
            // Se ainda houver frames livres, a página entra sem substituição.
            if (size < frames) { memory[size] = page; freq[size] = 1; size++; status = "Preenchimento"; }
            else {
                // Escolhe o frame com menor frequência; se houver empate, escolhe o menor valor numérico.
                int nfu_idx = 0;
                for (int j = 1; j < frames; j++) {
                    if (freq[j] < freq[nfu_idx]) nfu_idx = j;
                    else if (freq[j] == freq[nfu_idx] && memory[j] < memory[nfu_idx]) nfu_idx = j;
                }
                memory[nfu_idx] = page; freq[nfu_idx] = 1; trocas++; status = "Troca";
            }
        }

        // O estado da memória após cada referência ajuda a visualizar o impacto do histórico de acessos.
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

    // O número retornado corresponde apenas às substituições efetivas.
    return trocas;
}