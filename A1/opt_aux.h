// Trata de funcoes auxiliares das funcoes de operacoes.

#ifndef OPT_AUX_H
#define OPT_AUX_H

#include "utils.h"
#include <stdio.h>

// Insere os dados, de tamanho tam, do membro no archiver.
// Retorno: 0 em caso de sucesso e -1 c.c.
int insere_membro_arq(FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, unsigned long tam, int pos);

// Funcao que trata do caso do membro ja existir no archive.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip_mem_existe (struct diretorio * diretorio, char * membro, char * archive, int pos);


#endif