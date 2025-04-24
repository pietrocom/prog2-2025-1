#ifndef VINA_H
#define VINA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// ---- Defines ----

#define NOME_MAX 512
#define CAPACIDADE_DIR 20


// ---- Structs ----

struct arquivo {
    char nome[NOME_MAX];    
    int uid;       
    int tam_or;          
    int tam_comp;         
    FILE *arq;
    int ordem; 
    unsigned long offset;       
};

struct diretorio {
    // Vetor de structs no diretorio
    struct arquivo* membros;
    int qtd_membros;
    int capacidade;
};

// ---- Funcoes ----

// Aloca arquivo
struct arquivo * cria_s_arquivo ();

// Desaloca arquivo
void destroi_s_arquivo (struct arquivo * file);

// Aloca diretorio
struct diretorio * cria_diretorio ();

// Desaloca diretorio
void destroi_diretorio (struct diretorio * diretorio);

// Insere metadados do arquivo na lista
void insere_arquivo (struct diretorio * diretorio, struct arquivo * arq, FILE * file);

#endif