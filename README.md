# VINAc – Arquivador com Suporte à Compressão

## Autor
- Pietro Comin  
- GRR20241955  
- Contato: [pietro.comin@ufpr.br](mailto:pietro.comin@ufpr.br)
- Número de linhas: 1989

---

## Descrição

O **VINAc** é um arquivador com suporte à compressão baseado em linha de comando, semelhante a ferramentas como `tar`, `zip` ou `rar`. Seu objetivo é permitir o arquivamento de múltiplos arquivos em um único container (`.vc`) com opção de compressão individual usando o algoritmo LZ.  

A estrutura do arquivo `.vc` contém uma área de diretório com todas as informações de metadados necessárias para a manipulação dos membros armazenados.

---

## Funcionalidades

O programa `vinac` aceita as seguintes opções de execução:

- `-ip` ou `-p` : insere membros sem compressão  
- `-ic` ou `-i` : insere membros com compressão (LZ)  
- `-m membro` : move um membro dentro do archive  
- `-x` : extrai membros (ou todos)  
- `-r` : remove membros do archive  
- `-c` : lista o conteúdo do archive  

---

## Estrutura do Projeto

```
A1/
│
├── include/
│   ├── aux.h
│   ├── lz/
│   │   └── lz.h
│   ├── options.h
│   ├── types.h
│   ├── utils.h
│   └── vina.h
│
├── src/
│   ├── aux.c
│   ├── main.c
│   ├── options.c
│   ├── utils.c
│   ├── vina.c
│   └── lz/
│       └── lz.c
│
├── build/
│   └── objects/  (gerado durante a compilação, contém arquivos .o)
│
├── login/
│   └── vinac  (executável gerado)
│
├── makefile
├── A1 - O Arquivador VINAc.pdf  (enunciado do trabalho)
├── texto.txt
├── texto2.txt
├── texto3.txt
└── texto4.txt
```

---

## Compilação

Para compilar o projeto, utilize:

```bash
make
```

Para limpar os arquivos gerados:

```bash
make clean
```

---

## Estruturas de Dados e Algoritmos

### Estruturas de Dados

#### `struct arquivo`

Contém os metadados de cada membro do archive:

- `nome`  
- `UID`  
- `tamanho original`  
- `tamanho comprimido`  
- `ordem`  
- `offset no arquivo`  
- `data de modificação`  

#### `struct diretorio`

Gerencia todos os membros do archive como um vetor de ponteiros para `arquivo`.

---

## Modularização e Funções Relevantes

A manipulação de arquivos variáveis (em tamanho e posição) foi um dos maiores desafios do projeto. Para contornar isso, desenvolvi funções utilitárias modulares que permitiram abstrair a lógica de movimentação de dados com segurança:

```c
int move(...)                 // Move um bloco dentro do arquivo
int move_sequencial(...)      // Move membros em sequência, evitando sobrescrita
int insere_membro_arq(...)    // Insere dados de um membro no archive
int comprime_arquivo(...)     // Comprime membro com LZ e atualiza metadados
int descomprime_arquivo(...) // Descomprime membro e restaura seu conteúdo
```

Essas funções foram fundamentais para garantir consistência e evitar sobrescrita indesejada de bytes durante operações como inserção, movimentação e remoção.

---

## Decisões de Implementação

- A compressão LZ foi usada conforme exigido. Se o resultado da compressão não reduzir o tamanho, o membro é armazenado sem compressão.  
- A manipulação da área de diretório foi feita totalmente em memória RAM para facilitar o gerenciamento dos metadados.  
- Todos os dados dos membros são manipulados diretamente em disco, conforme restrição do enunciado.  
- As operações foram fortemente modularizadas para facilitar testes e manutenção.  

---

## Dificuldades Encontradas

- **Gerenciamento de deslocamentos:** mover blocos binários com offsets distintos sem sobrescrever dados foi desafiador. A criação das funções `move()` e `move_sequencial()` foi essencial.  
- **Compressão seletiva:** adaptar a lógica para armazenar membros comprimidos apenas quando vantajoso; exigiu lógica condicional detalhada no momento da inserção.  
- **Atualização de offsets:** cada operação (remoção, movimentação, inserção) exigia atualização precisa de todos os campos de offset e ordem no diretório. Movimentações podem apenas ocorrer de membro em membro, evitando buffers muito grandes.
- **Manter atomicidade:** como só se pode manipular um membro por vez, houve um esforço para otimizar leitura/escrita sem arquivos temporários ou buffers desnecessários.  

---

## Bugs Conhecidos

- Warning do Valgrind sobre bytes não inicializados. Foram feitas diversas sessões de debug sem suucesso. Pesquisas mostraram que talvez o erro seja inofensivo.
- Ainda não há verificação extensiva para desempenho em compressões muito grandes ou contendo caracteres inválidos.  

---

## Contato

Para dúvidas ou sugestões, entre em contato pelo e-mail: [pietro.comin@ufpr.br](mailto:pietro.comin@ufpr.br)
