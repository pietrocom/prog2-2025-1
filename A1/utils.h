#ifndef UTILS_H
#define UTILS_H

// ---- DEFINES ----

#define CAPACIDADE_DIR 20


// ---- INCLUDES ----

#include "vina.h"
#include <sys/stat.h>
#include <unistd.h>


// ---- FUNCOES ----

// O arquivo ja deve estar aberto.
// Move os bytes contidos entre inicio e fim, dentro de file, deslocamento bytes.
// Retorno: 0 para sucesso ou -1 para erro de operacao.
int move (unsigned long inicio, unsigned long fim, long deslocamento, FILE * file);

#endif