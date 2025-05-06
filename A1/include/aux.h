// Trata de funcoes auxiliares das funcoes de operacoes.

#ifndef AUX_H
#define AUX_H

#include "types.h"
#include <stdio.h>


// Funcao que trata do caso do membro ja existir no archive.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip_existe (struct diretorio * diretorio, char * membro, char * archive, int pos);

// Funcao que tambem trata do caso do membro ja existir no archive.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ic_existe (struct diretorio * diretorio, char * membro, char * archive, int pos);


#endif