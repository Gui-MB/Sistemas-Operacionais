# Escalonador de Processos

Este projeto implementa um simulador de escalonamento de processos em C. Ele lê um conjunto de processos de um arquivo de entrada e aplica o algoritmo de escalonamento escolhido sobre eles.

## Estrutura do projeto

- `main.c`: é o arquivo principal que carrega a entrada, aciona as simulações e realiza a contagem do tempo decorrido.
- `entradaEscalonador.txt`: é o arquivo de entrada que indica a estrutura dos processos e deve estar presente na raiz do projeto.
- `auxiliary_files/`: define os utilitários comuns (como a lógica de processos e a árvore rubro-negra).
- `scheduler_algorithms/`: implementações dos algoritmos de escalonamento.
- `input_generator/`: contém o arquivo gerador de entradas.

## Como compilar e executar
Na raiz do projeto:

```bash
gcc main.c auxiliary_files/* scheduler_algorithms/*.c -o escalonador
./escalonador
```

## Arquivo de entrada

O simulador lê os processos do arquivo em `entradaEscalonador.txt`. Cada linha representa um processo com seus atributos.

Para gerar entradas novas, use `input_files/geradorEntrada.py` (o arquivo será gerado na raiz do projeto).

## Algoritmos implementados

- **Alternância Circular** (`round_robin.c`).
- **Prioridade** (`priority.c`)l.
- **Lottery** (`lottery.c`).
- **CFS** (`cfs.c`).

## Funcionamento (visão geral)
1. O programa lê todos os processos de `entradaEscalonador.txt`.
2. Cada processo é inserido nas estruturas internas.
3. A simulação avança em passos de tempo, selecionando o próximo processo a executar conforme o algoritmo.
4. A cada decisão, o simulador atualiza o tempo de CPU consumido, espera, prioridade/dinâmica e estado do processo.
5. Ao final, o simulador exibe o resumo da execução e as métricas calculadas.

## Saída

Os resultados são exibidos no terminal. Em geral, a saída inclui:
- o algoritmo escolhido e o quantum,
- a ordem de execução dos processos ao longo do tempo,
- tempos de latência e espera por processo,
- métricas agregadas.

## Observações sobre IA

A IA foi utilizada para preencher os arquivos .h, para implementar a árvore vermelho e preta e para escrever o arquivo README.md