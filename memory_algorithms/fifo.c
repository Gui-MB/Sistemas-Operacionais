#include "fifo.h"
#include "../auxiliary_files/processes.h"

int fifo_simulate(int frames, const int *sequence, int seq_len, int is_global) {
    // A FIFO mantém as páginas na ordem em que foram carregadas.
    // Quando a memória enche, a página mais antiga é a primeira candidata à substituição.
    int memory[frames], size = 0, head = 0, trocas = 0;

    if (log_cfg.memory_steps) log_printf("\n--- Simulação FIFO ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Verifica se a página já está presente em memória.
        int page = sequence[i], found = 0;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = 1; break; } }

        const char *status;
        if (!found) {
            // Enquanto houver frames livres, apenas preenchemos a memória.
            if (size < frames) { memory[size++] = page; status = "Preenchimento"; }
            // Quando não há mais espaço, substituímos a página apontada pelo ponteiro FIFO.
            else { memory[head] = page; head = (head + 1) % frames; trocas++; status = "Troca"; }
        } else { status = "Acerto"; }

        // Se a configuração de logs estiver ativa, mostramos o estado completo após a referência.
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

    // O retorno representa apenas substituições reais; preenchimentos iniciais não contam como troca.
    return trocas;
}