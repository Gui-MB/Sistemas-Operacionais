#define _GNU_SOURCE // Extensões de busca em C
#include <stdio.h>
#include <stdlib.h>
#include <search.h> // Biblioteca para árvores balanceadas, incluse a rubro-negra

typedef enum { NEW, READY, RUNNING, TERMINATED } Estado;

typedef struct {
    int pid;
    int tempo_criacao, tempo_execucao, tempo_restante, prioridade;
    double vruntime;
    int tempo_conclusao;
    Estado estado;
} Processo;

/**
 * @brief Compara vruntime de dois processos
 * @param a: ponteiro para o primeiro processo
 * @param b: ponteiro para o segundo processo
 * @return -1 se p1 < p2, 1 se p1 > p2 e 0 se p1 == p2
 */
int comparar_processos(const void *a, const void *b) {
    const Processo *p1 = (const Processo *)a;
    const Processo *p2 = (const Processo *)b;

    if (p1 -> vruntime < p2 -> vruntime) return -1;
    if (p1 -> vruntime > p2 -> vruntime) return 1;
    return 0;}

/**
 * @brief Inicializa um processo com os parâmetros fornecidos e define seu estado como NEW
 * @param p: ponteiro para o processo a ser inicializado
 * @param tempo_criacao: tempo de criação do processo
 * @param pid: identificador único do processo
 * @param tempo_execucao: tempo total necessário para execução do processo
 * @param prioridade: prioridade do processo (quanto menor, mais alta a prioridade)
 * @return void
 */
static void inicializar_processo(Processo *p, int tempo_criacao, int pid,
                                 int tempo_execucao, int prioridade) {
    p->tempo_criacao = tempo_criacao;
    p->pid = pid;
    p->tempo_execucao = tempo_execucao;
    p->prioridade = prioridade;
    p->tempo_restante = tempo_execucao;
    p->vruntime = 0.0;
    p->estado = NEW;
}

/**
 * @brief Imprime as informações de um processo
 * @param p: ponteiro para o processo a ser impresso
 * @return void
 */
void imprimir_processo(const Processo *p) {
    printf("PID: %d, Tempo de Criação: %d, Tempo de Execução: %d, Prioridade: %d, Tempo Restante: %d, VRuntime: %.2f, Estado: %d\n",
           p->pid, p->tempo_criacao, p->tempo_execucao, p->prioridade,
           p->tempo_restante, p->vruntime, p->estado);
}