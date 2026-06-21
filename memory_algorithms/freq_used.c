#include "freq_used.h"
#include "../auxiliary_files/processes.h"

static int find_process_index_by_pid(int pid) {
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].pid == pid) return i;
    }
    return -1;
}

int nfu_simulate(int frames, const int *sequence, int seq_len, int is_global, int *trocas_por_processo) {
    // O NFU (Not Frequently Used) mantém um contador de uso por frame.
    // Na falta de página, sai a menos referenciada; em empate, usamos o menor ID.
    int memory[frames], freq[frames], size = 0, trocas = 0;
    int resident_count[MAX_PROCESSES] = {0};

    if (log_cfg.memory_steps) log_printf("\n--- Simulação NFU ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Primeiro verificamos se a página já está carregada.
        int page = sequence[i], found = -1;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = j; break; } }

        const char *status;
        // Em acerto, apenas incrementamos a frequência observada daquele frame.
        if (found != -1) { freq[found]++; status = "Acerto"; }
        else {
            int req_pid = is_global ? (page >> 16) : -1;
            int req_idx = is_global ? find_process_index_by_pid(req_pid) : -1;
            int req_limit = (req_idx >= 0) ? processes[req_idx].frame_limit : frames;
            int req_at_limit = is_global && req_idx >= 0 && resident_count[req_idx] >= req_limit;

            // Se ainda houver frames livres e o processo está abaixo do limite.
            if (size < frames && !req_at_limit) {
                memory[size] = page;
                freq[size] = 1;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;
                size++;
                status = "Preenchimento";
            } else {
                // Escolhe o frame com menor frequência entre os candidatos permitidos.
                int nfu_idx = -1;
                for (int j = 0; j < size; j++) {
                    if (req_at_limit && ((memory[j] >> 16) != req_pid)) continue;
                    if (nfu_idx == -1) {
                        nfu_idx = j;
                        continue;
                    }
                    if (freq[j] < freq[nfu_idx]) nfu_idx = j;
                    else if (freq[j] == freq[nfu_idx]) {
                        int page_j = memory[j] & 0xFFFF;
                        int page_best = memory[nfu_idx] & 0xFFFF;
                        if (page_j < page_best) nfu_idx = j;
                    }
                }

                if (nfu_idx == -1) nfu_idx = 0;

                if (is_global && size > 0) {
                    int victim_pid = memory[nfu_idx] >> 16;
                    int victim_idx = find_process_index_by_pid(victim_pid);
                    if (victim_idx >= 0) resident_count[victim_idx]--;
                }

                memory[nfu_idx] = page;
                freq[nfu_idx] = 1;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;

                trocas++;
                if (is_global && trocas_por_processo && req_idx >= 0) {
                    trocas_por_processo[req_idx]++;
                }
                status = "Troca";
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