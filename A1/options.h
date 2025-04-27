// Trata especificamente das funcoes de opcoes
// do archiver.

#ifndef OPTIONS_H
#define OPTIONS_H

#include "vina.h"
#include "utils.h"

// Funcao que trata da opcao -ip
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip (struct diretorio * diretorio, char * membro, char * archive);

#endif