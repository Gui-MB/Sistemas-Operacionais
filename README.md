# Simulador de Sistema Operacional

Este projeto implementa, em C, um simulador integrado e modularizado de um Sistema Operacional, abrangendo o escalonamento de processos na CPU, gerenciamento de substituição de páginas na memória e o escalonamento de operações de Entrada e Saída (E/S).

## Estrutura do projeto
- `main.c`: Arquivo principal que carrega a entrada, aciona as simulações e gerencia a intercalação entre CPU, Memória e Dispositivos de E/S.
- `entradaEscalonador.txt`: Arquivo de configuração e carga de processos.
- `saidaEscalonador.txt`: Arquivo de saída gerado que contém o log completo e o resumo da simulação.
- `processes/`: Gerenciador de processos (process_manager.c/process_manager.h), responsável por definir a estrutura de dados dos processos, manter o estado global e realizar a leitura e interpretação do arquivo de entrada.
- `logs/`: Sistema de formatação de logs e exibição do estado do sistema.
- `auxiliary_files/`: Define utilitários comuns, como por exemplo as estruturas de dados (árvores, heaps).
- `scheduler_algorithms/`: Algoritmos de escalonamento de CPU (Round Robin, Prioridade, Loteria, CFS).
- `memory_algorithms/`: Algoritmos de substituição de páginas (FIFO, LRU, NFU, Ótimo).
- `io_algorithms/`: Gerenciador de E/S (`io_manager.c`/`io_manager.h`) com controle de bloqueio e filas de espera circulares.

## Como compilar e executar
Na raiz do projeto, utilize o comando abaixo:
```bash
gcc main.c auxiliary_files/*.c scheduler_algorithms/*.c memory_algorithms/*.c io_algorithms/*.c logs/*.c -o escalonador
./escalonador
```

## Formato de entrada

A entrada deve estar em `entradaEscalonador.txt` e seguir o formato:

```text
algoritmoDeEscalonamento|fraçãoDeCPU|políticaMemória|tamanhoMemória|tamanhoPáginasMolduras|percentualAlocação|numDispositivosES
idDispositivo|numUsosSimultaneos|tempoOperação
...
tempoCriacaoProcesso|PID|tempoDeExecução|prioridade (ou bilhetes)|qtdeMemoria|sequênciaAcessoPaginasProcesso|chanceRequisitarES
```

Significado dos campos da primeira linha:

- `algoritmoDeEscalonamento`: `alternancia` (ou `alternanciaCircular`), `prioridade`, `loteria` ou `CFS`.
- `fraçãoDeCPU`: quantum de CPU usado pelo escalonador.
- `políticaMemória`: `local` ou `global`.
- `tamanhoMemória`: tamanho da memória principal em bytes.
- `tamanhoPáginasMolduras`: tamanho da página/moldura em bytes.
- `percentualAlocação`: percentual máximo de alocação por processo.
- `numDispositivosES`: quantidade de dispositivos de E/S do sistema, uma linha por dispositivo é lida em seguida.

Significado dos campos das linhas de dispositivo (uma por dispositivo de E/S):

- `idDispositivo`: identificador do dispositivo.
- `numUsosSimultaneos`: quantidade de processos que podem usar o dispositivo ao mesmo tempo.
- `tempoOperação`: tempo que o dispositivo demora para concluir uma operação de E/S.

Significado dos campos das demais linhas (um processo por linha):

- `tempoCriacaoProcesso`: instante de criação do processo.
- `PID`: identificador único do processo.
- `tempoDeExecução`: tempo total necessário de CPU.
- `prioridade (ou bilhetes)`: prioridade (ou quantidade de bilhetes na loteria).
- `qtdeMemoria`: memória virtual solicitada pelo processo (bytes).
- `sequênciaAcessoPaginasProcesso`: sequência de páginas referenciadas.
- `chanceRequisitarES`: chance (%) do processo solicitar uma operação de E/S durante sua fatia de CPU.

## Observações:

- Troca de página conta apenas substituição real; carregamento inicial não conta troca.
- No NUF, em empate de frequência, é escolhida a página de menor ID.
- Em cada ciclo de CPU, ocorre no máximo um acesso de memória por processo em execução.

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

## Observações sobre IA

A IA foi utilizada para:

- refatorar a arquitetura de logs
- refatorar a organização das informações dos processos para incluir memória
- refatorar a organização do algoritmo de CFS para permitir E/S
- ajuda para encontrar o problema que causava a memória "global" não funcionar
- criar a função `log_printf()`
- implementar a estrutura de dados da árvore vermelho e preta
- implementar a estrutura de dados heap mínimo
- revisar as informações dos arquivos .h
- auxiliar na escrita do arquivo README.md