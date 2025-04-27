// Trata de operacoes de manipulacao de arquivo e de TADs.

#ifndef UTILS_H
#define UTILS_H

// ---- DEFINES ----


// ---- INCLUDES ----

#include "types.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// ---- DECLARACOES AVANCADAS ----
struct arquivo; 
struct diretorio;


// ---- FUNCOES ----

// Insere os metadados do arquivo na struct arquivo.
// Retorno: a struct modificada ou NULL em caso de erro.
struct arquivo * inicia_valores_arquivo (struct arquivo * arquivo, char * nome);

// O arquivo ja deve estar aberto.
// Move os bytes contidos entre inicio e fim, dentro de file, deslocamento bytes.
// Retorno: 0 para sucesso ou -1 para erro de operacao.
int move (unsigned long inicio, unsigned long fim, long deslocamento, FILE * file);

// Move, membro por membro todos os membros a frente de pos. 
// Projetada para que nao sobrescreva informacao.
// A funcao nao eh na verdade iterativa, mas o nome reflete seu comportamento.
// Retorno: 0 em caso de sucesso ou -1 c.c.
int move_recursivo (struct diretorio * diretorio, FILE * archive_pt, int pos, long deslocamento);

// Insere a struct arquivo na posicao desejada do vetor de membros.
// Caso pos == -1, insere no final do vetor. 
// Retorno: 0 para sucesso ou -1 para erro de insercao.
int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arquivo, int pos);

// Passa por cada elemento do vetor e atualiza offset e ordem.
void atualiza_metadados (struct diretorio * diretorio);

// Escreve as informacoes contidas no diretorio no archiver.
// Retorno: 0 em caso de sucesso ou -1 c.c.
int escreve_s_diretorio (struct diretorio * diretorio, FILE * archive_pt);

#endif