#include "fifo.h"
#include "../auxiliary_files/processes.h"

static int find_process_index_by_pid(int pid) {
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].pid == pid) return i;
    }
    return -1;
}

int fifo_simulate(int frames, const int *sequence, int seq_len, int is_global, int *trocas_por_processo) {
    // A FIFO mantém as páginas na ordem em que foram carregadas.
    // Quando a memória enche, a página mais antiga é a primeira candidata à substituição.
    int memory[frames], size = 0, head = 0, trocas = 0;
    int resident_count[MAX_PROCESSES] = {0};

    if (log_cfg.memory_steps) log_printf("\n--- Simulação FIFO ---\n");

    for (int i = 0; i < seq_len; i++) {
        // Verifica se a página já está presente em memória.
        int page = sequence[i], found = 0;
        for (int j = 0; j < size; j++) { if (memory[j] == page) { found = 1; break; } }

        const char *status;
        if (!found) {
            int req_pid = is_global ? (page >> 16) : -1;
            int req_idx = is_global ? find_process_index_by_pid(req_pid) : -1;
            int req_limit = (req_idx >= 0) ? processes[req_idx].frame_limit : frames;
            int req_at_limit = is_global && req_idx >= 0 && resident_count[req_idx] >= req_limit;

            // Enquanto houver frames livres e o processo ainda está abaixo do limite, apenas preenchemos.
            if (size < frames && !req_at_limit) {
                memory[size++] = page;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;
                status = "Preenchimento";
            } else {
                int replace_idx = -1;

                // Em modo global, se o processo já atingiu o limite, ele só pode substituir páginas próprias.
                if (req_at_limit) {
                    for (int off = 0; off < size; off++) {
                        int idx = (head + off) % ((size > 0) ? size : 1);
                        if ((memory[idx] >> 16) == req_pid) {
                            replace_idx = idx;
                            break;
                        }
                    }
                }

                // Caso geral FIFO: usa a cabeça da fila circular.
                if (replace_idx == -1) {
                    replace_idx = (size > 0) ? head : 0;
                }

                if (is_global && size > 0) {
                    int victim_pid = memory[replace_idx] >> 16;
                    int victim_idx = find_process_index_by_pid(victim_pid);
                    if (victim_idx >= 0) resident_count[victim_idx]--;
                }

                memory[replace_idx] = page;
                if (is_global && req_idx >= 0) resident_count[req_idx]++;

                if (size > 0) head = (replace_idx + 1) % size;
                trocas++;
                if (is_global && trocas_por_processo && req_idx >= 0) {
                    trocas_por_processo[req_idx]++;
                }
                status = "Troca";
            }
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