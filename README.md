# Escalonador de Processos (Simulador)

Este projeto implementa um simulador de escalonamento de processos em C. Ele lê um conjunto de processos de um arquivo de entrada e aplica um determinado algoritmo de escalonamento sobre eles, apresentando os resultados de latência ao final.

## Estrutura do projeto

- `main.c`: ponto de entrada, carrega a entrada e aciona as simulações.
- `Auxiliary_Files/`: utilitários comuns (definição dos processos e da árvore rubro-negra).
- `Scheduler_Algorithms/`: implementações dos algoritmos de escalonamento.
- `Input_Files/`: arquivos de entrada e gerador.

## Como compilar e executar
Na raiz do projeto:

```bash
gcc main.c Auxiliary_Files/processes.c Auxiliary_Files/red_and_black_tree.c Scheduler_Algorithms/*.c -o escalonador
./escalonador
```

## Arquivo de entrada

O simulador lê os processos do arquivo em `Input_Files/entradaEscalonador.txt`. Cada linha representa um processo com seus atributos.

Para gerar entradas novas, use `Input_Files/geradorEntrada.py`.

## Algoritmos implementados

- **Round Robin** (`round_robin.c`).
- **Prioridade** (`priority.c`)l.
- **Lottery** (`lottery.c`).
- **CFS** (`cfs.c`).

## Funcionamento (visao geral)

1. O `main.c` carrega os processos e suas informacoes.
2. Para cada algoritmo, o simulador organiza a fila de prontos conforme as regras do escalonador.
3. O tempo de simulacao avanca, executando e atualizando cada processo.
4. Ao final, o simulador imprime o resultado, conforme a implementacao de cada algoritmo.

## Saida

Os resultados sao exibidos no terminal. O formato exato pode variar entre os algoritmos, mas tipicamente inclui a ordem de execucao e metricas como tempo de espera, tempo de resposta e turnaround.

## Observacoes sobre IA

A IA foi utilizada para preencher os arquivos .h, na implementação da árvore vermelho e preta e para escrever o arquivo README.md