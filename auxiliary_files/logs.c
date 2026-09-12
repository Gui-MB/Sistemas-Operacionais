#include "logs.h"
#include "devices.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../memory_algorithms/memory_manager.h"
#include "../memory_algorithms/fifo.h"
#include "../memory_algorithms/rec_used.h"
#include "../memory_algorithms/freq_used.h"
#include "../memory_algorithms/optimal.h"

// Inicialização das configurações de Log
LogConfig log_cfg = {
    .cpu_events = 1,
    .memory_steps = 1,
    .final_metrics = 1
};

static FILE *log_file = NULL;

int init_output_file(const char *filename) {
    log_file = fopen(filename, "w");
    if (!log_file) return 0;
    return 1;
}

void close_output_file(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

// Imprime no arquivo, de saída, e retorna o número de caracteres escritos
int log_printf(const char *format, ...) {
    int count = 0;
    if (log_file) {
        va_list args_file;
        va_start(args_file, format);
        count = vfprintf(log_file, format, args_file);
        va_end(args_file);
        fflush(log_file);
    }
    return count;
}

static void print_algo_details(const Process *p) {
    if (strcmp(algorithm, "prioridade") == 0) log_printf(" | priority=%-4d", p->priority);
    else if (strcmp(algorithm, "loteria") == 0) log_printf(" | tickets=%-4d", p->priority);
    else if (strcmp(algorithm, "CFS") == 0) log_printf(" | priority=%-4d | vruntime=%-4d", p->priority, p->vruntime);
}

void print_process_event(const char *event, int current_time, const Process *p, int run_time) {
    if (!log_cfg.cpu_events) return; 

    log_printf("[T=%03d] %-7s | pid=%-3d", current_time, event, p->pid);
    if (strcmp(event, "CREATE") == 0) log_printf(" | total_t=%-8d", p->exec_time);
    else if (strcmp(event, "RUN") == 0) log_printf(" | remaining_t=%-4d | cpu_slice=%-3d", p->remaining_time, run_time);
    else if (strcmp(event, "PREEMPT") == 0) log_printf(" | remaining_t=%-4d", p->remaining_time);
    else if (strcmp(event, "FINISH") == 0) log_printf(" | remaining_t=%-4d", p->remaining_time);
    print_algo_details(p);
    log_printf("\n");
}

void announce_created_processes(int current_time) {
    for (int i = 0; i < num_processes; i++) {
        if (!processes[i].creation_announced && processes[i].creation_time <= current_time) {
            processes[i].creation_announced = 1;
            print_process_event("CREATE", current_time, &processes[i], 0);
        }
    }
}

// Imprime o estado completo do sistema a cada troca de processo na CPU: quem executa, quem está pronto,
// quem está bloqueado (e em qual dispositivo) e o estado de todos os dispositivos de E/S.
void print_system_state(int current_time, int running_idx) {
    log_printf("\n[T=%03d] ---- Estado do sistema ----\n", current_time);

    if (running_idx >= 0 && running_idx < num_processes) {
        Process *p = &processes[running_idx];
        log_printf("  Executando: PID %d (restante=%d)\n", p->pid, p->remaining_time);
    } else {
        log_printf("  Executando: (CPU ociosa)\n");
    }

    log_printf("  Prontos:");
    int any_ready = 0;
    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];
        if (i == running_idx) continue;
        if (p->is_completed) continue;
        if (p->creation_time > current_time) continue;
        if (p->state != STATE_READY) continue;
        log_printf(" PID %d(restante=%d)", p->pid, p->remaining_time);
        any_ready = 1;
    }
    if (!any_ready) log_printf(" nenhum");
    log_printf("\n");

    log_printf("  Bloqueados:");
    int any_blocked = 0;
    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];
        if (p->state != STATE_BLOCKED) continue;
        // Consulta o estado REAL do dispositivo (não a cópia guardada no processo no momento do
        // pedido), pois o processo pode ter sido promovido da fila para um slot livre desde então.
        int is_waiting = 0;
        int dev_idx = device_status_of_pid(p->pid, &is_waiting);
        const char *dev_id = (dev_idx >= 0) ? devices[dev_idx].id : "-1";
        log_printf(" PID %d(restante=%d, dispositivo=%s, %s)", p->pid, p->remaining_time, dev_id,
                   is_waiting ? "aguardando" : "em uso");
        any_blocked = 1;
    }
    if (!any_blocked) log_printf(" nenhum");
    log_printf("\n");

    devices_print_state();
}

void print_metrics_scaling(void) {
    if (log_cfg.final_metrics) {
        log_printf("\n----------------------- RESULTADOS DA EXECUÇÃO -----------------------\n");
        log_printf("%-5s | %-16s | %-16s | %-16s | %-16s\n",
                   "PID", "Tempo Total", "Tempo Pronto", "Tempo Bloqueado", "Tempo Execução");
        log_printf("-----------------------------------------------------------------------------\n");
    }

    float total_turnaround = 0, total_ready = 0, total_blocked = 0;
    int total_exec_time = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        int turnaround_time = p.completion_time - p.creation_time;

        total_turnaround += turnaround_time;
        total_ready += p.ready_time_accum;
        total_blocked += p.blocked_time_accum;
        total_exec_time += p.exec_time;

        if (log_cfg.final_metrics) {
            log_printf("%-5d | %-16d | %-16d | %-16d | %-16d\n",
                       p.pid, turnaround_time, p.ready_time_accum, p.blocked_time_accum, p.exec_time);
        }
    }

    if (log_cfg.final_metrics) {
        log_printf("-----------------------------------------------------------------------------\n");
        log_printf("Turnaround Médio (criação->conclusão): %-16.2f\n", total_turnaround / num_processes);
        log_printf("Tempo Pronto Médio: %-16.2f\n", total_ready / num_processes);
        log_printf("Tempo Bloqueado Médio: %-16.2f\n", total_blocked / num_processes);
        log_printf("Tempo Total de Execução: %-16d\n", total_exec_time);
    }
}

void print_metrics_memory(void) {
    int total_fifo = 0, total_lru = 0, total_nfu = 0, total_optimal = 0;
    int fifo_by_proc[MAX_PROCESSES] = {0};
    int lru_by_proc[MAX_PROCESSES] = {0};
    int nfu_by_proc[MAX_PROCESSES] = {0};
    int optimal_by_proc[MAX_PROCESSES] = {0};

    // Lógica para Memória GLOBAL
    if (strcmp(memory_policy, "global") == 0) {
        int total_frames = 0;
        int physical_frames = memory_size_bytes / page_size_bytes;
        if (physical_frames <= 0) physical_frames = 1;

        // Em modo global, o limite de frames considera a soma do teto de cada processo.
        // Isso evita superalocar molduras e mascarar trocas com valores sempre zerados.
        for (int i = 0; i < num_processes; i++) {
            total_frames += processes[i].frame_limit;
        }
        if (total_frames <= 0) total_frames = 1;
        if (total_frames > physical_frames) total_frames = physical_frames;

        if (log_cfg.memory_steps) {
            log_printf("\n----------------------------------------------------------------------\n");
            log_printf("SIMULANDO MEMÓRIA GLOBAL (Frames Totais: %d)\n", total_frames);
            log_printf("----------------------------------------------------------------------\n");
        }

        total_fifo    = fifo_simulate(total_frames, global_page_sequence, global_sequence_len, 1, fifo_by_proc);
        total_lru     = lru_simulate(total_frames, global_page_sequence, global_sequence_len, 1, lru_by_proc);
        total_nfu     = nfu_simulate(total_frames, global_page_sequence, global_sequence_len, 1, nfu_by_proc);
        total_optimal = optimal_simulate(total_frames, global_page_sequence, global_sequence_len, 1, optimal_by_proc);

    // Lógica para Memória LOCAL
    } else {
        for (int i = 0; i < num_processes; i++) {
            PageFaultResult res;
            memory_manager_simulate(&processes[i], &res);
            fifo_by_proc[i] = res.fifo_faults;
            lru_by_proc[i] = res.lru_faults;
            nfu_by_proc[i] = res.nfu_faults;
            optimal_by_proc[i] = res.optimal_faults;
            total_fifo    += res.fifo_faults;
            total_lru     += res.lru_faults;
            total_nfu     += res.nfu_faults;
            total_optimal += res.optimal_faults;
        }
    }

    int diff_fifo = abs(total_fifo - total_optimal);
    int diff_lru  = abs(total_lru  - total_optimal);
    int diff_nfu  = abs(total_nfu  - total_optimal);

    int min_diff = diff_fifo;
    if (diff_lru < min_diff) min_diff = diff_lru;
    if (diff_nfu < min_diff) min_diff = diff_nfu;

    int count_best = 0;
    const char *best = "";

    if (diff_fifo == min_diff) { count_best++; best = "FIFO"; }
    if (diff_lru == min_diff)  { count_best++; best = "LRU"; }
    if (diff_nfu == min_diff)  { count_best++; best = "NFU"; }

    if (count_best > 1) best = "empate";

    if (log_cfg.final_metrics) {
        log_printf("\n----------------------- RESULTADOS DA MEMÓRIA  ----------------------\n");
        log_printf("%-8s | %-8s | %-8s | %-8s | %-8s\n", "FIFO", "LRU", "NFU", "OTM", "Melhor");
        log_printf("----------------------------------------------------------------------\n");
        log_printf("%-8d | %-8d | %-8d | %-8d | %-8s\n", total_fifo, total_lru, total_nfu, total_optimal, best);
        log_printf("----------------------------------------------------------------------\n");

        log_printf("\n-------------------- TROCAS DE MEMÓRIA POR PROCESSO/ALGORITMO --------------------\n");
        log_printf("%-6s | %-8s | %-8s | %-8s | %-8s\n", "PID", "FIFO", "LRU", "NFU", "OTM");
        log_printf("----------------------------------------------------------------------\n");
        for (int i = 0; i < num_processes; i++) {
            log_printf("%-6d | %-8d | %-8d | %-8d | %-8d\n",
                      processes[i].pid,
                      fifo_by_proc[i],
                      lru_by_proc[i],
                      nfu_by_proc[i],
                      optimal_by_proc[i]);
        }
        log_printf("----------------------------------------------------------------------\n");
    }

    // Saída principal em uma linha para correção automática.
    printf("%d|%d|%d|%d|%s\n", total_fifo, total_lru, total_nfu, total_optimal, best);

    // Tabela adicional no terminal com trocas por processo e por algoritmo.
    printf("%-6s | %-8s | %-8s | %-8s | %-8s\n", "PID", "FIFO", "LRU", "NFU", "OTM");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%-6d | %-8d | %-8d | %-8d | %-8d\n",
               processes[i].pid,
               fifo_by_proc[i],
               lru_by_proc[i],
               nfu_by_proc[i],
               optimal_by_proc[i]);
    }
}
