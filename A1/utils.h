// Trata de operacoes de manipulacao de arquivo e de TADs.

#ifndef UTILS_H
#define UTILS_H

// ---- DEFINES ----


// ---- INCLUDES ----

#include "types.h"
#include <stdio.h>


// ---- FUNCOES ----

// O arquivo ja deve estar aberto.
// Move os bytes contidos entre inicio e fim, dentro de file, deslocamento bytes.
// Retorno: 0 para sucesso ou -1 para erro de operacao.
int move (unsigned long inicio, unsigned long fim, long deslocamento, FILE * file);

// Move, membro por membro todos os membros a frente de pos, alterando o offset.
// Projetada para que nao sobrescreva informacao.
// A funcao nao eh na verdade iterativa, mas o nome reflete seu comportamento.
// Retorno: 0 em caso de sucesso ou -1 c.c.
int move_recursivo (struct diretorio * diretorio, FILE * archive_pt, int pos, long deslocamento);

// Insere os dados, de tamanho tam, do membro, em pos, no archiver.
// Se a struct arquivo for passada, atualiza seu offset.
// Retorno: 0 em caso de sucesso e -1 c.c.
int insere_membro_arq (FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, struct arquivo * arquivo, unsigned long tam, int pos);

// Comprime um arquivo e muda campo de tam_comp da struct.
// Retorno: 0 em caso de sucesso e -1 c.c.
int comprime_arquivo (char * file_name, FILE * file_pt, struct arquivo * arquivo);

// Imprime os metadados da struct arquivo passada.
void imprime_arquivo_info (struct arquivo * arquivo);

#endif