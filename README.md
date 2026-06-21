# Simulador de Escalonador de Processos e Gerenciador de Memória

Este projeto implementa, em C, um simulador integrado de escalonamento de processos e substituição de páginas.

O programa lê `entradaEscalonador.txt`, simula a execução dos processos ciclo a ciclo e compara os algoritmos de memória **FIFO**, **LRU**, **NUF (NFU)** e **Ótimo** pelo número de trocas de página.

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

O arquivo `saidaEscalonador.txt` é gerado automaticamente com o log detalhado.

## Formato de entrada

A entrada deve estar em `entradaEscalonador.txt` e seguir o formato:

```text
algoritmoDeEscalonamento|fraçãoDeCPU|políticaMemória|tamanhoMemória|tamanhoPáginasMolduras|percentualAlocação
tempoCriacaoProcesso|PID|tempoDeExecução|prioridade (ou bilhetes)|qtdeMemoria|sequênciaAcessoPaginasProcesso
```

Significado dos campos da primeira linha:

- `algoritmoDeEscalonamento`: `alternancia` (ou `alternanciaCircular`), `prioridade`, `loteria` ou `CFS`.
- `fraçãoDeCPU`: quantum de CPU usado pelo escalonador.
- `políticaMemória`: `local` ou `global`.
- `tamanhoMemória`: tamanho da memória principal em bytes.
- `tamanhoPáginasMolduras`: tamanho da página/moldura em bytes.
- `percentualAlocação`: percentual máximo de alocação por processo.

Significado dos campos das demais linhas (um processo por linha):

- `tempoCriacaoProcesso`: instante de criação do processo.
- `PID`: identificador único do processo.
- `tempoDeExecução`: tempo total necessário de CPU.
- `prioridade (ou bilhetes)`: prioridade (ou quantidade de bilhetes na loteria).
- `qtdeMemoria`: memória virtual solicitada pelo processo (bytes).
- `sequênciaAcessoPaginasProcesso`: sequência de páginas referenciadas.

## Formato de saída

Ao final da execução, o programa imprime **uma linha com resultados e uma tabela com as trocas de memória**:

```text
38|40|35|27|NFU
PID    | FIFO     | LRU      | NFU      | OTM   
---------------------------------------------
1      | 7        | 8        | 7        | 5   
2      | 9        | 11       | 11       | 8   
3      | 22       | 21       | 17       | 14  
```

Onde:

- `FIFO`, `LRU`, `NUF`, `OTIMO`: número de trocas de página de cada algoritmo.
- `melhor`: algoritmo com desempenho mais próximo do ótimo (`FIFO`, `LRU`, `NFU`), ou `empate` em caso de empate.

Observações de implementação:

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

## Funcionamento (visão geral)

1. O programa lê todos os processos de `entradaEscalonador.txt`.
2. Cada processo é inserido nas estruturas internas.
3. A simulação avança em passos de tempo, selecionando o próximo processo a executar conforme o algoritmo.
4. A cada decisão, o simulador atualiza o tempo de CPU consumido, espera, prioridade/dinâmica e estado do processo.
5. Ao final, o simulador exibe o resumo da execução e as métricas calculadas.

## Saída detalhada

Além da linha final para correção automática, o arquivo `saidaEscalonador.txt` contém logs de escalonamento, passos de memória e tabelas-resumo por processo/algoritmo.

## Observações sobre IA

A IA foi utilizada para:

- refatorar a arquitetura de logs
- log em formato de tabela para número de trocas por processo e algoritmo
- ajuda para encontrar o problema que causava o "global" não funcionar
- refatorar a organização das informações dos processos para incluir memória
- criar a função `log_printf()`
- implementar a estrutura de dados da árvore vermelho e preta
- implementar a estrutura de dados heap mínimo
- revisar as informações dos arquivos .h
- auxiliar na escrita do arquivo README.md
