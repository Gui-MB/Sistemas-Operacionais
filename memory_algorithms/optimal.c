#include "optimal.h"
#include "../auxiliary_files/processes.h"

static int find_process_index_by_pid(int pid) {
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].pid == pid) return i;
    }
    return -1;
}

int optimal_simulate(int frames, const int *sequence, int seq_len, int is_global, int *trocas_por_processo) {
    // O algoritmo ótimo olha o futuro da sequência e remove a página cujo próximo uso está mais distante.
    // Ele serve como referência teórica para comparar os demais algoritmos.
    int memory[frames], size = 0, trocas = 0;
    int resident_count[MAX_PROCESSES] = {0};

    if (log_cfg.memory_steps) log_printf("\n--- Simulação ÓTIMO ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Verifica se a referência atual já está na memória antes de decidir qualquer substituição.
        int page = sequence[i], found = 0;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = 1; break; } }

        const char *status;
        if (!found) {
            int req_pid = is_global ? (page >> 16) : -1;
            int req_idx = is_global ? find_process_index_by_pid(req_pid) : -1;
            int req_limit = (req_idx >= 0) ? processes[req_idx].frame_limit : frames;
            int req_at_limit = is_global && req_idx >= 0 && resident_count[req_idx] >= req_limit;

            // Enquanto houver molduras livres e o processo está abaixo do limite, a página é apenas carregada.
            if (size < frames && !req_at_limit) {
                memory[size++] = page;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;
                status = "Preenchimento";
            } else {
                // Para cada página candidata, procura a próxima ocorrência no restante da sequência.
                int replace_idx = -1, farthest_next_use = -1;
                for (int j = 0; j < size; j++) {
                    if (req_at_limit && ((memory[j] >> 16) != req_pid)) continue;

                    int next_use = -1;
                    for (int k = i + 1; k < seq_len; k++) {
                        if (sequence[k] == memory[j]) { next_use = k; break; }
                    }
                    // Se a página nunca mais será usada, ela é a melhor candidata para remoção.
                    if (next_use == -1) { replace_idx = j; break; }
                    // Caso todas serão reutilizadas, removemos a que será usada mais tarde.
                    if (next_use > farthest_next_use) { farthest_next_use = next_use; replace_idx = j; }
                }

                if (replace_idx == -1) replace_idx = 0;

                if (is_global && size > 0) {
                    int victim_pid = memory[replace_idx] >> 16;
                    int victim_idx = find_process_index_by_pid(victim_pid);
                    if (victim_idx >= 0) resident_count[victim_idx]--;
                }

                memory[replace_idx] = page;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;

                trocas++;
                if (is_global && trocas_por_processo && req_idx >= 0) {
                    trocas_por_processo[req_idx]++;
                }
                status = "Troca";
            }
        } else { status = "Acerto"; }

        // O log detalhado deixa explícito quando o algoritmo explora conhecimento do futuro.
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

    // O ótimo também só contabiliza substituições reais, sem contar o carregamento inicial.
    return trocas;
}