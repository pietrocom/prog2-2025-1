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


// ---- FUNCOES ----

// O arquivo ja deve estar aberto.
// Move os bytes contidos entre inicio e fim, dentro de file, deslocamento bytes.
// Retorno: 0 para sucesso ou -1 para erro de operacao.
int move (unsigned long inicio, unsigned long fim, long deslocamento, FILE * file);

// Move, membro por membro todos os membros a frente de pos, alterando o offset.
// Projetada para que nao sobrescreva informacao.
// A funcao nao eh na verdade iterativa, mas o nome reflete seu comportamento.
// Retorno: 0 em caso de sucesso ou -1 c.c.
int move_recursivo (struct diretorio * diretorio, FILE * archive_pt, int pos, long deslocamento);

// Insere os dados, de tamanho tam, do membro, em pos, no archiver.
// Retorno: 0 em caso de sucesso e -1 c.c.
int insere_membro_arq (FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, unsigned long tam, int pos);

#endif