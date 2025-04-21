#ifndef UTILS_H
#define UTILS_H

// ---- DEFINES ----

#define CAPACIDADE_DIR 10


// ---- INCLUDES ----

#include "vina.h"
#include "fprio.h"
#include <sys/stat.h>
#include <unistd.h>


// ---- FUNCOES ----

// Aloca arquivo
struct arquivo * cria_arquivo ();

// Desaloca arquivo
void destroi_arquivo (struct arquivo * file);

// Aloca diretorio
struct diretorio * cria_diretorio ();

// Desaloca diretorio
void destroi_diretorio (struct diretorio * diretorio);

// Insere metadados do arquivo na lista
void insere_arquivo (struct diretorio * diretorio, struct arquivo * file);

#endif