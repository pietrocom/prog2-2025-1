// Trata especificamente das funcoes de opcoes do archiver.

#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"
#include <stdio.h>

// Funcao que trata da opcao -ip.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip (struct diretorio * diretorio, char * membro, char * archive, struct arquivo * novo_arq);

// Funcao que trata da opcao -ic.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ic (struct diretorio * diretorio, char * membro, char * archive);

// Funcao que trata da opcao -m.
// Retorno: 0 em caso de sucesso e -1 c.c
int m (struct diretorio * diretorio, char * membro, char * target, char * archive);

#endif