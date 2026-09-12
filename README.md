# Trabalho OS 3 — Escalonador com Gerenciador de E/S

Extensão do simulador de escalonamento (Alternância Circular) e memória (política Local) para
incorporar um **gerenciador de dispositivos de Entrada/Saída (E/S)**, conforme o enunciado do
Trabalho OS 3.

## Como compilar

```bash
gcc -Wall -Wextra -o escalonador main.c auxiliary_files/*.c scheduler_algorithms/*.c memory_algorithms/*.c
```

## Como executar

```bash
./escalonador
```

O programa lê `entradaEscalonador.txt` (mesmo diretório do executável) e grava o log completo em
`saidaEscalonador.txt`. No terminal, imprime apenas o resumo final de trocas de página por algoritmo
de memória (para fins de correção automática).

## Formato do arquivo de entrada

```
algoritmoDeEscalonamento|fraçãoDeCPU|políticaMemória|tamanhoMemória|tamanhoPáginasMolduras|percentualAlocação|numDispositivosES
idDispositivo|numUsosSimultaneos|tempoOperação
...  (uma linha por dispositivo, numDispositivosES linhas no total)
tempoCriação|PID|tempoDeExecução|prioridade|qtdeMemoria|sequênciaAcessoPaginasProcesso|chanceRequisitarES
...  (uma linha por processo)
```

- `algoritmoDeEscalonamento`: nesta entrega, apenas `alternancia` (Alternância Circular) é utilizado.
- `políticaMemória`: nesta entrega, apenas `local` é utilizada.
- `numDispositivosES`: quantidade de dispositivos de E/S do sistema.
- `idDispositivo`: identificador do dispositivo (usado nos logs).
- `numUsosSimultaneos`: quantos processos podem usar o dispositivo ao mesmo tempo.
- `tempoOperação`: quantos ciclos de tempo uma operação de E/S naquele dispositivo demora.
- `chanceRequisitarES`: chance (0–100) do processo requisitar uma operação de E/S a cada vez que é
  escalonado para a CPU. Se o processo decidir requisitar, o dispositivo e o instante dentro da fatia
  de CPU em que isso ocorre são sorteados aleatoriamente.

Exemplo (`entradaEscalonador.txt`):

```
alternancia|1|local|65536|512|50|2
1|1|3
2|2|5
0|1|20|59|4096|1 2 2 2 3 4 3 4 5 5 6 1 5 3 2 6 7 7 7 8|30
0|2|24|32|2048|1 2 2 2 3 4 3 4 4 4 2 3 2 1 3 2 1 2 2 3 4 3 2 2|20
0|3|32|32|4096|1 2 3 4 5 6 7 8 4 3 2 1 1 6 7 5 6 8 3 2 2 1 2 2 4 4 5 3 2 1 7 8|10
```

## Comportamento do gerenciador de E/S

- Ao ser escalonado, cada processo sorteia (com base em `chanceRequisitarES`) se vai requisitar uma
  operação de E/S durante a fatia de CPU que recebeu. Se sim, sorteia também qual dispositivo e em
  qual ciclo da fatia isso ocorre.
- Se o dispositivo tiver um slot livre (`numUsosSimultaneos` respeitado), o processo começa a operação
  imediatamente. Caso contrário, entra na fila de espera (FIFO) do dispositivo.
- Enquanto está bloqueado (operando ou aguardando), o processo não é escalonado para a CPU. Os demais
  processos prontos continuam concorrendo à CPU normalmente pela alternância circular.
- Quando a operação de E/S termina, o processo volta ao estado "pronto" e é reenfileirado no final da
  fila de prontos (comportamento padrão de alternância circular).

## Saída (`saidaEscalonador.txt`)

A cada troca de processo na CPU, o log mostra:

```
[T=XXX] ---- Estado do sistema ----
  Executando: PID <pid> (restante=<tempo>)
  Prontos: PID <pid>(restante=<tempo>) ...
  Bloqueados: PID <pid>(restante=<tempo>, dispositivo=<id>, em uso|aguardando) ...
  Dispositivos:
    Dispositivo <id> (slots=<n>, tempo_operacao=<t>): [uso: PID <pid>, restante=<t>] ... | fila: PID <pid> ...
```

Eventos de CPU (`CREATE`, `RUN`, `PREEMPT`, `FINISH`, `IO_REQ`, `IO_DONE`) também são registrados
linha a linha.

Ao final, a tabela de resultados mostra, para cada processo: tempo total (criação → conclusão),
tempo total em estado "pronto" e tempo total em estado "bloqueado" (aguardando/realizando E/S), além
do tempo de execução efetivo na CPU.

A simulação de memória (FIFO/LRU/NFU/Ótimo) é mantida como no trabalho anterior, para fins de
referência e comparação, mesmo com apenas a política `local` sendo usada nesta entrega.

## Estrutura dos arquivos

| Arquivo | Descrição |
|---|---|
| `main.c` | Loop principal: simulação ciclo a ciclo da CPU, integrando o escalonador e o gerenciador de E/S |
| `auxiliary_files/processes.h/c` | Estrutura `Process`, leitura do arquivo de entrada (agora incluindo dispositivos e `chanceRequisitarES`) |
| `auxiliary_files/devices.h/c` | **Novo**: gerenciador de dispositivos de E/S (slots simultâneos, fila de espera, `device_tick`) |
| `auxiliary_files/logs.h/c` | Impressão de eventos, estado do sistema a cada troca de contexto e métricas finais |
| `scheduler_algorithms/round_robin.c` | Alternância Circular, adaptada para respeitar processos bloqueados por E/S |
| `scheduler_algorithms/scheduler_manager.c` | Seleciona o algoritmo de escalonamento configurado |
| `memory_algorithms/*` | Simulação de FIFO, LRU, NFU e Ótimo (mantidos do trabalho anterior) |

Os módulos `priority.c`, `lottery.c`, `cfs.c` (e suas estruturas de apoio `heap_min`,
`red_and_black_tree`) permanecem no projeto por compatibilidade, mas não são utilizados nesta entrega,
já que o enunciado pede a consideração de apenas um algoritmo de escalonamento (Alternância Circular)
e um de memória (Local).
