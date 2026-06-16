#include "logs.h"
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

void print_metrics_scaling(void) {
    if (log_cfg.final_metrics) {
        log_printf("\n----------------------- RESULTADOS DA EXECUÇÃO -----------------------\n");
        log_printf("%-5s | %-17s | %-16s | %-16s\n", "PID", "Latência", "Tempo de Espera", "Tempo de Execução");
        log_printf("----------------------------------------------------------------------\n");
    }

    float total_latency = 0, total_wt = 0;
    int total_exec_time = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        int latency_time = p.completion_time - p.creation_time;
        int waiting_time = latency_time - p.exec_time;

        total_latency += latency_time;
        total_wt += waiting_time;
        total_exec_time += p.exec_time;

        if (log_cfg.final_metrics) {
            log_printf("%-5d | %-16d | %-16d | %-16d\n", p.pid, latency_time, waiting_time, p.exec_time);
        }
    }

    if (log_cfg.final_metrics) {
        log_printf("----------------------------------------------------------------------\n");
        log_printf("Latência Média: %-16.2f\n", total_latency / num_processes);
        log_printf("Tempo de Espera Médio: %-16.2f\n", total_wt / num_processes);
        log_printf("Tempo Total de Execução: %-16d\n", total_exec_time);
    }
}

void print_metrics_memory(void) {
    int total_fifo = 0, total_lru = 0, total_nfu = 0, total_optimal = 0;

    // Lógica para Memória GLOBAL
    if (strcmp(memory_policy, "global") == 0) {
        int total_frames = memory_size_bytes / page_size_bytes;
        if (total_frames <= 0) total_frames = 1;

        if (log_cfg.memory_steps) {
            log_printf("\n----------------------------------------------------------------------\n");
            log_printf("SIMULANDO MEMÓRIA GLOBAL (Frames Totais: %d)\n", total_frames);
            log_printf("----------------------------------------------------------------------\n");
        }

        total_fifo    = fifo_simulate(total_frames, global_page_sequence, global_sequence_len, 1);
        total_lru     = lru_simulate(total_frames, global_page_sequence, global_sequence_len, 1);
        total_nfu     = nfu_simulate(total_frames, global_page_sequence, global_sequence_len, 1);
        total_optimal = optimal_simulate(total_frames, global_page_sequence, global_sequence_len, 1);

    // Lógica para Memória LOCAL
    } else {
        for (int i = 0; i < num_processes; i++) {
            PageFaultResult res;
            memory_manager_simulate(&processes[i], &res);
            total_fifo    += res.fifo_faults;
            total_lru     += res.lru_faults;
            total_nfu     += res.nfu_faults;
            total_optimal += res.optimal_faults;
        }
    }

    int diff_fifo = total_fifo - total_optimal;
    int diff_lru  = total_lru  - total_optimal;
    int diff_nfu  = total_nfu  - total_optimal;

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
        log_printf("\n----------------------- RESULTADOS DA MEMÓRIA  -----------------------\n");
        log_printf("%-8s | %-8s | %-8s | %-8s | %-8s\n", "FIFO", "LRU", "NFU", "OTM", "Melhor");
        log_printf("----------------------------------------------------------------------\n");
        log_printf("%-8d | %-8d | %-8d | %-8d | %-8s\n", total_fifo, total_lru, total_nfu, total_optimal, best);
        log_printf("----------------------------------------------------------------------\n");
    }

    // SAÍDA EXCLUSIVA DO TERMINAL (Usando printf nativo)
    printf("%d|%d|%d|%d|%s\n", total_fifo, total_lru, total_nfu, total_optimal, best);
}