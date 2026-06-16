# Simulador de Escalonador de Processos e Gerenciador de Memória

Este projeto implementa um simulador integrado de escalonamento de processos e gerenciamento de memória em C. Ele lê um conjunto de processos de um arquivo de entrada e aplica algoritmos de escalonamento de CPU a algoritmos de substituição de páginas na memória.

O projeto foi testado e implementado usando a versão 11.4.0 do GCC (Ubuntu 11.4.0-1ubuntu1~22.04.3).

## Estrutura do projeto

- `main.c`: Arquivo principal que carrega a entrada, aciona as simulações, realiza a contagem do tempo decorrido e gerencia a intercalação entre CPU e Memória.
- `entradaEscalonador.txt`: Arquivo de entrada que indica a configuração do sistema e a estrutura dos processos. Deve estar na raiz do projeto.
- `scheduler_algorithms/`: Implementações dos algoritmos de escalonamento de CPU.
- `memory_algorithms/`: Implementações dos algoritmos de substituição de páginas de memória.
- `auxiliary_files/`: Define utilitários comuns, como estruturas de dados (árvores, heaps) e o sistema de formatação de logs (`prints.c`/`prints.h`).
- `input_generator/`: Contém os scripts em Python geradores de entradas somente para o processo de escalonador.
- `saidaEscalonador.txt`: Arquivo de saída gerado que contém o log completo e o resumo da simulação.

## Como compilar e executar

Na raiz do projeto, utilize o comando abaixo (certifique-se de compilar todos os arquivos `.c` das subpastas):

```bash
gcc main.c auxiliary_files/*.c scheduler_algorithms/*.c memory_algorithms/*.c -o escalonador
./escalonador
```

## Algoritmos de escalonamento implementados

- **Alternância Circular** (`round_robin.c`).
- **Prioridade** (`priority.c`).
- **Lottery** (`lottery.c`).
- **CFS** (`cfs.c`).

## Políticas de memória implementadas

- **Local:** Cada processo substitui apenas suas próprias páginas dentro de um limite de frames calculado pelo seu percentual de alocação.
- **Global:** Todos os processos competem pelas mesmas molduras de memória física, sofrendo e causando substituições de forma intercalada de acordo com os ciclos de CPU.

## Algoritmos de memória implementados

- **FIFO (First-In-First-Out)** (`fifo.c`).
- **LRU (Menos Recentemente Usada)** (`rec_used.c`).
- **NFU (Não Usada Frequentemente)** (`freq_used.c`).
- **Ótimo** (`optimal.c`).

## Funcionamento (visão geral)

1. O programa lê todos os processos de `entradaEscalonador.txt`.
2. Cada processo é inserido nas estruturas internas.
3. A simulação avança em passos de tempo, selecionando o próximo processo a executar conforme o algoritmo.
4. A cada decisão, o simulador atualiza o tempo de CPU consumido, espera, prioridade/dinâmica e estado do processo.
5. Ao final, o simulador exibe o resumo da execução e as métricas calculadas.

## Saída

Os resultados são exibidos no terminal e também gravados em `saidaEscalonador.txt`.

## Observações sobre IA

A IA foi utilizada para:

- refatorar a arquitetura de logs
- refatorar a organização das informações dos processos para incluir memória
- criar a função `log_printf()`
- implementar a estrutura de dados da árvore vermelho e preta
- implementar a estrutura de dados heap mínimo
- revisar as informações dos arquivos .h
- auxiliar na escrita do arquivo README.md