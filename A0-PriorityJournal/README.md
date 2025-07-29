# A0: Simulador de Redação de Jornal

**Disciplina:** Programação 2
**Período:** 2025/1
**Autor:** Pietro Comin (GRR20241955)

## 📰 Sobre o Projeto

Este projeto consiste na implementação de um sistema para gerenciar o fluxo de notícias em uma redação de jornal. O programa simula a chegada de notícias e a montagem de edições, utilizando um sistema de prioridades para diferenciar "breaking news" (urgentes) de notícias comuns.

O objetivo foi aplicar conceitos de Tipos Abstratos de Dados (TADs) e manipulação de listas ligadas em C para criar uma estrutura de dados funcional.

## ✔️ Funcionalidades

O sistema é baseado em duas filas principais, uma para cada nível de prioridade:

* **Cadastro de Notícias:** O usuário pode inserir o título e o texto de uma nova notícia, definindo sua prioridade (0 para breaking news, 1 para normal).
* **Filas de Prioridade:** As notícias são inseridas na fila correspondente à sua prioridade.
* **Seleção para Edição:** Ao "fechar uma edição", o sistema seleciona as notícias de maior prioridade disponíveis (breaking news sempre antes das normais).
* **Ciclo de Vida da Notícia:** As notícias "envelhecem" a cada edição e são descartadas automaticamente após 3 dias (edições) para manter o conteúdo relevante.

## 🛠️ Tecnologias Utilizadas

* **Linguagem:** C
* **Estrutura de Dados:** O sistema foi implementado utilizando **duas filas baseadas em listas ligadas**, uma para cada nível de prioridade, para garantir a ordem de chegada (FIFO) dentro de cada categoria.

## 🚀 Como Compilar e Executar

O projeto pode ser compilado diretamente com o GCC. Navegue até a pasta do projeto e execute:

```bash
gcc -o jornal_sim *.c -lm
./jornal_sim
