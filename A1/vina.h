// Pietro Comin - GRR20241955
// Trata das operacoes iniciais para que se possa realizar
// as opcoes no archiver.

#ifndef VINA_H
#define VINA_H


// ---- INCLUDES ----

#include "types.h"
#include <stdio.h>


// ---- FUNCOES DE ARQUIVO E STRUCT ARQUIVO ----

// Cria e retorna um ponteiro para um arquivo.vc de nome file,
// ou NULL em caso de erro.
FILE * cria_arquivo (char * file);

// Aloca struct arquivo.
// Retorno: a struct ou NULL em caso de erro.
struct arquivo * cria_s_arquivo ();

// Insere os metadados do arquivo na struct arquivo.
// Retorno: a struct modificada ou NULL em caso de erro.
struct arquivo * inicia_s_arquivo (struct arquivo * arquivo, char * nome);

// Insere a struct arquivo na posicao desejada do vetor de membros.
// Caso pos == -1, insere no final do vetor. 
// Retorno: 0 para sucesso ou -1 para erro de insercao.
int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arquivo, int pos);

// Passa por cada elemento do vetor e atualiza offset e ordem.
void atualiza_metadados (struct diretorio * diretorio);

// Desaloca struct arquivo.
void destroi_s_arquivo (struct arquivo * file);



// ---- FUNCOES DE STRUCT DIRETORIO ----

// Aloca struct diretorio.
// Retorno: a struct ou NULL em caso de erro.
struct diretorio * cria_diretorio ();

// Desaloca struct diretorio.
void destroi_diretorio (struct diretorio * diretorio);

// Acessa o archiver e extrai informacoes do diretorio.
// Retorno: 0 em caso de sucesso e -1 c.c.
int inicia_diretorio (struct diretorio * diretorio, char * file);

// Escreve as informacoes contidas no diretorio no archiver.
// Retorno: 0 em caso de sucesso ou -1 c.c.
int escreve_s_diretorio (struct diretorio * diretorio, FILE * archive_pt);

#endif