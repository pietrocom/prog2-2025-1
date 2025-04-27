// Trata de operacoes de manipulacao de arquivo e de TADs.

#ifndef UTILS_H
#define UTILS_H

// ---- DEFINES ----


// ---- INCLUDES ----

#include "vina.h"
#include <sys/stat.h>
#include <unistd.h>


// ---- FUNCOES ----

// Insere os metadados do arquivo na struct arquivo.
// Retorno: a struct modificada ou NULL em caso de erro.
struct arquivo * inicia_valores_arquivo (struct arquivo * arquivo, char * nome);

// O arquivo ja deve estar aberto.
// Move os bytes contidos entre inicio e fim, dentro de file, deslocamento bytes.
// Retorno: 0 para sucesso ou -1 para erro de operacao.
int move (unsigned long inicio, unsigned long fim, long deslocamento, FILE * file);

// Insere a struct arquivo na posicao desejada do vetor de membros.
// Caso pos == -1, insere no final do vetor. 
// Retorno: 0 para sucesso ou -1 para erro de insercao.
int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arquivo, int pos);

#endif