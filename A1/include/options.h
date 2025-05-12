// Trata especificamente das funcoes de opcoes do archiver.

#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"
#include <stdio.h>

// Funcao que trata da opcao -ip.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip (struct diretorio * diretorio, char * membro, char * archive);

// Funcao que trata da opcao -ic.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ic (struct diretorio * diretorio, char * membro, char * archive);

// Funcao que trata da opcao -m.
// Retorno: 0 em caso de sucesso e -1 c.c.
int m (struct diretorio * diretorio, char * membro, char * target, char * archive);

// Funcao que trata da opcao -x.
// Retorno: 0 em caso de sucesso e -1 c.c.
int x (struct diretorio * diretorio, char * membro, char * archive);

// Funcao que trata da opcao -r.
// Retorno: 0 em caso de sucesso e -1 c.c.
int r (struct diretorio * diretorio, char * file_name, char * archive);

// Funcao que trata da opcao -r.
// Retorno: 0 em caso de sucesso e -1 c.c.
void c (struct diretorio * diretorio);

#endif