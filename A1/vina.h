#ifndef VINA_H
#define VINA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// ---- Defines ----

#define NOME_MAX 512


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
    // Vetor de ponteiros para structs no diretorio
    struct arquivo **membros;
    int qtd_membros;
};

// ---- Funcoes ----

// Aloca struct arquivo.
// Retorno: a struct ou NULL em caso de erro.
struct arquivo * cria_s_arquivo ();

// Desaloca struct arquivo.
void destroi_s_arquivo (struct arquivo * file);

// Aloca struct diretorio.
// Retorno: a struct ou NULL em caso de erro.
struct diretorio * cria_diretorio ();

// Desaloca struct diretorio.
void destroi_diretorio (struct diretorio * diretorio);

// Insere metadados do arquivo no diretorio.
// Retorno: 0 caso sucesso e -1 c.c.
int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arq, FILE * file);

#endif