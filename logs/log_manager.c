#include "log_manager.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../processes/process_manager.h"
#include "../memory_algorithms/memory_manager.h"
#include "../io_algorithms/io_manager.h"
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

// Variáveis estáticas para separar a simulação do print da memória
static int mem_simulated = 0;
static int total_fifo = 0, total_lru = 0, total_nfu = 0, total_optimal = 0;
static int fifo_by_proc[MAX_PROCESSES] = {0};
static int lru_by_proc[MAX_PROCESSES] = {0};
static int nfu_by_proc[MAX_PROCESSES] = {0};
static int optimal_by_proc[MAX_PROCESSES] = {0};

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

// Imprime no arquivo, no terminal, e retorna o número de caracteres escritos no arquivo
int log_printf(const char *format, ...) {
    int count = 0;
    va_list args_stdout;
    va_start(args_stdout, format);

    if (log_file) {
        va_list args_file;
        va_copy(args_file, args_stdout);
        count = vfprintf(log_file, format, args_file);
        va_end(args_file);
        fflush(log_file);
    }

    vprintf(format, args_stdout);
    va_end(args_stdout);
    return count;
}

static void print_algo_details(const Process *p) {
    if (strcmp(algorithm, "prioridade") == 0) log_printf(" | priority=%-4d", p->priority);
    else if (strcmp(algorithm, "loteria") == 0) log_printf(" | tickets=%-4d", p->priority);
    else if (strcmp(algorithm, "CFS") == 0) log_printf(" | priority=%-4d | vruntime=%-4d", p->priority, p->vruntime);
}

void print_process_event(const char *event, int current_time, const Process *p, int run_time) {
    if (!log_cfg.cpu_events) return; 

    log_printf("\n[T=%04d] %-7s | pid=%-3d", current_time, event, p->pid);
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

// Imprime o estado atual do sistema (processos e dispositivos) a cada troca de contexto
void print_system_state(int current_time, int running_idx) {
    if (!log_cfg.cpu_events) return;

    log_printf("  [T=%04d]\n    System State: \n", current_time);

    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];
        if (!p->creation_announced || p->is_completed) continue;

        if (i == running_idx) {
            log_printf("      RUNNING | pid=%-3d | remaining_t=%-4d\n", p->pid, p->remaining_time);
        } else if (p->blocked) {
            log_printf("      BLOCKED | pid=%-3d | remaining_t=%-4d | io_device=%s (%s)\n",
                       p->pid, p->remaining_time, devices[p->requested_device_id].name,
                       p->device_in_use ? "using" : "waiting");
        } else {
            log_printf("      READY   | pid=%-3d | remaining_t=%-4d\n", p->pid, p->remaining_time);
        }
    }

    io_manager_print_state();
}

// Garante que a simulação de memória execute e imprima os passos ANTES das tabelas finais
static void ensure_memory_simulated(void) {
    if (mem_simulated) return; // Evita rodar duas vezes
    mem_simulated = 1;

    // Lógica para Memória GLOBAL
    if (strcmp(memory_policy, "global") == 0) {
        int total_frames = 0;
        int physical_frames = memory_size_bytes / page_size_bytes;
        if (physical_frames <= 0) physical_frames = 1;

        for (int i = 0; i < num_processes; i++) {
            total_frames += processes[i].frame_limit;
        }
        if (total_frames <= 0) total_frames = 1;
        if (total_frames > physical_frames) total_frames = physical_frames;

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
}

void print_metrics_scaling(void) {
    // Força a simulação e os prints passo-a-passo da memória ocorrerem agora
    ensure_memory_simulated();

    if (log_cfg.final_metrics) {
        log_printf("\n----------------------- RESULTADOS DA EXECUÇÃO -----------------------\n");
        log_printf("%-5s | %-10s | %-16s | %-16s | %-14s\n", "PID", "Latência", "Tempo em Espera", "Tempo Bloqueado", "Tempo na CPU");
        log_printf("----------------------------------------------------------------------\n");
    }

    float total_latency = 0, total_wt = 0, total_bt = 0;
    int total_exec_time = 0;
    int tempo_simulacao = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        int latency_time = p.completion_time - p.creation_time;
    
        total_latency += latency_time;
        total_wt += p.ready_wait_time;
        total_bt += p.blocked_time;
        total_exec_time += p.exec_time;

        if (p.completion_time > tempo_simulacao) {
            tempo_simulacao = p.completion_time;
        }

        if (log_cfg.final_metrics) {
            log_printf("%-5d | %-9d | %-16d | %-16d | %-14d\n", p.pid, latency_time, p.ready_wait_time, p.blocked_time, p.exec_time);
        }
    }

    int tempo_idle = tempo_simulacao - total_exec_time;
    float ocupacao_cpu = ((float)total_exec_time / tempo_simulacao) * 100.0;

    if (log_cfg.final_metrics) {
        log_printf("----------------------------------------------------------------------\n");
        log_printf("Tempo de Latência Médio: %-16.2f\n", total_latency / num_processes);
        log_printf("Tempo de Espera Médio: %-16.2f\n", total_wt / num_processes);
        log_printf("Tempo de Bloqueio Médio: %-16.2f\n", total_bt / num_processes);
        log_printf("Tempo de Execução Médio: %-16.2f\n", (float)total_exec_time / num_processes);
        log_printf("Taxa de Ocupação da CPU: %.2f%%\n", ocupacao_cpu);
        log_printf("Tempo Total em Espera (Pronto): %-14d\n", (int)total_wt);
        log_printf("Tempo Total em Bloqueado: %-14d\n", (int)total_bt);
        log_printf("Tempo Total da CPU ociosa (Idle): %-14d\n", tempo_idle);
        log_printf("Tempo Total em Execução: %-14d\n", total_exec_time);
        log_printf("Tempo Total de Simulação: %-14d\n", tempo_simulacao);
    }
}

void print_metrics_memory(void) {
    // A simulação já ocorreu em print_metrics_scaling, mas chamamos para garantir segurança
    ensure_memory_simulated();

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
        log_printf("\n----------------------- RESULTADOS DA MEMÓRIA  -----------------------\n");
        log_printf("%-8s | %-8s | %-8s | %-8s | %-8s\n", "FIFO", "LRU", "NFU", "OTM", "Melhor");
        log_printf("----------------------------------------------------------------------\n");
        log_printf("%-8d | %-8d | %-8d | %-8d | %-8s\n", total_fifo, total_lru, total_nfu, total_optimal, best);
        log_printf("----------------------------------------------------------------------\n");

        log_printf("\n---------------- TROCAS DE MEMÓRIA POR PROCESSO/ALGORITMO ------------\n");
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
        
        // Saída principal em uma linha para correção automática.
        printf("%d|%d|%d|%d|%s\n", total_fifo, total_lru, total_nfu, total_optimal, best);
    }
}

void print_metrics_io(void) {
    if (!log_cfg.final_metrics) return;

log_printf("\n------------------------------------------- RESULTADOS DE E/S -------------------------------------------\n");
    log_printf("%-5s | %-13s | %-16s | %-16s | %-17s | %-15s\n", 
               "PID", "Total de Reqs", "Tempo Bloqueado", "Uso Efetivo E/S", "Tempo Fila E/S", "% Vida em Fila");
    log_printf("---------------------------------------------------------------------------------------------------------\n");

    int total_sys_requests = 0;
    int total_sys_blocked = 0;
    int total_sys_queue = 0;

    for (int i = 0; i < num_processes; i++) {
        Process p = processes[i];
        
        // O tempo bloqueado menos o tempo de fila dá o tempo real em que a máquina trabalhou
        int uso_efetivo = p.blocked_time - p.io_queue_time;
        
        // Porcentagem da vida do processo perdida APENAS na fila de dispositivo
        int lifetime = p.completion_time - p.creation_time;
        float percent_queue = lifetime > 0 ? ((float)p.io_queue_time / lifetime) * 100.0f : 0.0f;
        
        total_sys_requests += p.io_request_count;
        total_sys_blocked += p.blocked_time;
        total_sys_queue += p.io_queue_time;

        log_printf("%-5d | %-13d | %-16d | %-16d | %-17d | %-14.2f%%\n", 
                   p.pid, p.io_request_count, p.blocked_time, uso_efetivo, p.io_queue_time, percent_queue);
    }

    log_printf("---------------------------------------------------------------------------------------------------------\n");
    log_printf("Total de Solicitações E/S: %-14d\n", total_sys_requests);
    log_printf("Tempo Total Bloqueado (Fila + Uso): %-14d\n", total_sys_blocked);
    log_printf("Tempo Total Perdido em Filas de E/S: %-14d\n", total_sys_queue);
}