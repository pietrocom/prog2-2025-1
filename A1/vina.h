// Pietro Comin - GRR20241955
// Trata das operacoes iniciais para que se possa realizar
// as opcoes no archiver.

#ifndef VINA_H
#define VINA_H


// ---- INCLUDES ----

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "types.h"


// ---- Funcoes ----

// Cria e retorna um ponteiro para um arquivo.vc de nome file,
// ou NULL em caso de erro.
FILE * cria_arquivo (char * file);

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

// Acessa o archiver e extrai informacoes do diretorio.
// Retorno: 0 em caso de sucesso e -1 c.c.
int inicia_diretorio (struct diretorio * diretorio, char * file);

#endif