#include "rec_used.h"
#include "../auxiliary_files/processes.h"

static int find_process_index_by_pid(int pid) {
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].pid == pid) return i;
    }
    return -1;
}

int lru_simulate(int frames, const int *sequence, int seq_len, int is_global, int *trocas_por_processo) {
    // A LRU substitui a página menos recentemente usada.
    // Para isso, armazenamos o instante do último acesso de cada frame.
    int memory[frames], last_used[frames], size = 0, trocas = 0;
    int resident_count[MAX_PROCESSES] = {0};

    if (log_cfg.memory_steps) log_printf("\n--- Simulação LRU ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Procura a página na memória para distinguir acerto de falta.
        int page = sequence[i], found = -1;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = j; break; } }

        const char *status;
        // Em caso de acerto, atualizamos o instante do último uso.
        if (found != -1) { last_used[found] = i; status = "Acerto"; }
        else {
            int req_pid = is_global ? (page >> 16) : -1;
            int req_idx = is_global ? find_process_index_by_pid(req_pid) : -1;
            int req_limit = (req_idx >= 0) ? processes[req_idx].frame_limit : frames;
            int req_at_limit = is_global && req_idx >= 0 && resident_count[req_idx] >= req_limit;

            // Ainda existe capacidade livre e o processo está abaixo do limite.
            if (size < frames && !req_at_limit) {
                memory[size] = page;
                last_used[size] = i;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;
                size++;
                status = "Preenchimento";
            } else {
                // Escolhe o frame com menor timestamp entre os candidatos permitidos.
                int lru_idx = -1;
                for (int j = 0; j < size; j++) {
                    if (req_at_limit && ((memory[j] >> 16) != req_pid)) continue;
                    if (lru_idx == -1 || last_used[j] < last_used[lru_idx]) lru_idx = j;
                }

                if (lru_idx == -1) lru_idx = 0;

                if (is_global && size > 0) {
                    int victim_pid = memory[lru_idx] >> 16;
                    int victim_idx = find_process_index_by_pid(victim_pid);
                    if (victim_idx >= 0) resident_count[victim_idx]--;
                }

                memory[lru_idx] = page;
                last_used[lru_idx] = i;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;

                trocas++;
                if (is_global && trocas_por_processo && req_idx >= 0) {
                    trocas_por_processo[req_idx]++;
                }
                status = "Troca";
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