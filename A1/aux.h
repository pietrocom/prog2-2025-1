// Trata de funcoes auxiliares das funcoes de operacoes.

#ifndef AUX_H
#define AUX_H

#include "types.h"
#include "utils.h"
#include "vina.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Funcao que trata do caso do membro ja existir no archive.
// Retorno: 0 em caso de sucesso e -1 c.c.
int ip_mem_existe (struct diretorio * diretorio, char * membro, char * archive, int pos);


#endif