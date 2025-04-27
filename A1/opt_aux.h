// Trata de funcoes auxiliares das funcoes de operacoes.

#ifndef OPT_AUX_H
#define OPT_AUX_H

#include "utils.h"
#include <stdio.h>

// Funcao que trata do caso do membro ja existir no archive.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip_mem_igual (struct diretorio * diretorio, char * membro, char * archive, int pos);


#endif