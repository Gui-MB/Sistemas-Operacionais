#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "auxiliary_files/processes.h"
#include "auxiliary_files/devices.h"
#include "scheduler_algorithms/cfs.h"
#include "scheduler_algorithms/scheduler_manager.h"
#include "scheduler_algorithms/lottery.h"
#include "scheduler_algorithms/round_robin.h"
#include "memory_algorithms/memory_manager.h"

#define INPUT_FILE "entradaEscalonador.txt"
#define OUTPUT_FILE "saidaEscalonador.txt"

// Processa os PIDs que tiveram sua operação de E/S concluída neste ciclo: volta o processo para
// o estado PRONTO e o reenfileira no escalonador (alternância circular).
static void handle_finished_io(int current_time) {
    for (int f = 0; f < finished_count; f++) {
        int fpid = finished_pids[f];
        for (int j = 0; j < num_processes; j++) {
            if (processes[j].pid == fpid) {
                processes[j].state = STATE_READY;
                processes[j].device_index = -1;
                processes[j].waiting_in_queue = 0;
                log_printf("[T=%03d] IO_DONE | pid=%-3d | operacao de E/S concluida\n", current_time, fpid);
                rr_enqueue_unblocked(j);
                break;
            }
        }
    }
}

// Contabiliza, para todo processo já criado e ainda não concluído (exceto o que está rodando agora),
// mais um ciclo de tempo no estado em que ele se encontra (pronto ou bloqueado).
static void accumulate_wait_times(int current_time, int running_idx) {
    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];
        if (p->is_completed) continue;
        if (p->creation_time > current_time) continue;
        if (i == running_idx) continue;

        if (p->state == STATE_BLOCKED) {
            p->blocked_time_accum++;
        } else if (p->state == STATE_READY) {
            p->ready_time_accum++;
        }
    }
}

// Sorteia, no início de uma fatia de CPU, se o processo vai requisitar E/S nesta fatia,
// qual dispositivo será solicitado e em qual ciclo (relativo ao início da fatia) isso ocorrerá.
static void roll_io_request(Process *p, int slice_len) {
    p->io_will_request = 0;
    p->io_trigger_tick = -1;
    p->io_device_choice = -1;

    if (num_devices <= 0 || slice_len <= 0) return;

    int roll = rand() % 100;
    if (roll < p->chance_requisitar_es) {
        p->io_will_request = 1;
        p->io_trigger_tick = rand() % slice_len;
        p->io_device_choice = rand() % num_devices;
    }
}

// Executa o processo selecionado ciclo a ciclo, podendo ser interrompido a qualquer momento
// da fatia por um pedido de E/S. Imprime o estado do sistema toda vez que a CPU troca de processo.
static void execute_process(int selected_idx, int *current_time, int *completed_processes) {
    Process *p = &processes[selected_idx];
    int slice_len = (p->remaining_time > time_slice) ? time_slice : p->remaining_time;

    print_process_event("RUN", *current_time, p, slice_len);
    p->state = STATE_RUNNING;

    // A CPU acabou de trocar para este processo: mostra o estado completo do sistema agora
    print_system_state(*current_time, selected_idx);

    roll_io_request(p, slice_len);

    for (int tick = 0; tick < slice_len; tick++) {
        // Cada ciclo de CPU contabiliza um acesso real à memória
        if (p->next_access_index < p->page_sequence_len) {
            int page = p->page_sequence[p->next_access_index++];
            record_memory_access(p->pid, page);
        }

        p->remaining_time--;
        (*current_time)++;

        // Atualiza o vruntime para o CFS (mantido por compatibilidade, embora não usado nesta entrega)
        if (scheduler_manager_get_algorithm() == ALG_CFS) {
            p->vruntime += p->priority;
        }

        // Contabiliza mais um ciclo de espera (pronto/bloqueado) para os demais processos,
        // usando o estado de cada processo tal como estava DURANTE este ciclo (antes de qualquer
        // desbloqueio de E/S que só passa a valer no próximo ciclo).
        accumulate_wait_times(*current_time, selected_idx);

        // Avança 1 ciclo global nos dispositivos de E/S e libera quem concluiu operação
        device_tick();
        handle_finished_io(*current_time);

        // O processo terminou sua execução durante este ciclo
        if (p->remaining_time <= 0) {
            p->is_completed = 1;
            p->completion_time = *current_time;
            (*completed_processes)++;
            print_process_event("FINISH", *current_time, p, 0);
            return;
        }

        // Chegou o instante sorteado para este processo requisitar E/S nesta fatia
        if (p->io_will_request && tick == p->io_trigger_tick) {
            int dev_idx = p->io_device_choice;
            int started_now = device_request(dev_idx, p->pid);

            p->state = STATE_BLOCKED;
            p->device_index = dev_idx;
            p->waiting_in_queue = started_now ? 0 : 1;

            log_printf("[T=%03d] IO_REQ  | pid=%-3d | dispositivo=%-10s | %s\n",
                       *current_time, p->pid, devices[dev_idx].id,
                       started_now ? "iniciou operacao" : "entrou na fila de espera");
            return;
        }
    }

    // A fatia terminou sem que o processo tenha concluído ou requisitado E/S: preempção normal
    p->state = STATE_READY;
    print_process_event("PREEMPT", *current_time, p, 0);
}

int main(void) {
    srand((unsigned)time(NULL));

    // Configuração de logs
    log_cfg.cpu_events = 1;
    log_cfg.memory_steps = 1;
    log_cfg.final_metrics = 1;

    if (!init_output_file(OUTPUT_FILE)) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída %s!\n", OUTPUT_FILE);
        return 1;
    }
    // Lê o arquivo de entrada e inicializa processos e dispositivos
    read_input_file(INPUT_FILE);

    // Inicializa o gerenciador de escalonamento
    scheduler_manager_init();

    if (log_cfg.cpu_events) {
        log_printf("\n----------------------------------------------------------------------\n");
        log_printf("Log de escalonamento:\n");
        log_printf("Algoritmo: %s | Slice: %d | Dispositivos de E/S: %d\n\n", algorithm, time_slice, num_devices);
    }

    // Loop principal de simulação do escalonamento
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < num_processes) {
        announce_created_processes(current_time);

        int had_error = 0;
        int selected_idx = scheduler_manager_select_next(current_time, &had_error);
        if (had_error) return 1;

        if (selected_idx == -1) {
            if (log_cfg.cpu_events) log_printf("[T=%03d] IDLE\n", current_time);

            // O tempo avança 1 ciclo mesmo com a CPU ociosa, na mesma ordem usada em execute_process:
            // primeiro o relógio global avança, depois contabilizamos pronto/bloqueado com o estado
            // "antigo", e só então os dispositivos avançam (podendo desbloquear processos para o
            // próximo ciclo).
            current_time++;
            accumulate_wait_times(current_time, -1);
            device_tick();
            handle_finished_io(current_time);
            print_system_state(current_time, -1);

            continue;
        }

        execute_process(selected_idx, &current_time, &completed_processes);

        // Para o algoritmo de loteria, é necessário informar que o processo terminou para atualizar a segment tree
        if (scheduler_manager_get_algorithm() == ALG_LOTTERY) {
            lottery_process_finished(selected_idx);
        }
    }

    // Destrói a estrutura de árvore no caso do CFS
    if (scheduler_manager_get_algorithm() == ALG_CFS) {
        cfs_destroy();
    }

    print_metrics_scaling();
    print_metrics_memory();
    
    close_output_file();
    return 0;
}
