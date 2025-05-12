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

// Move, membro por membro todos os membros a frente de pos e antes de fim, alterando o offset.
// Projetada para que nao sobrescreva informacao.
// A funcao nao eh na verdade iterativa, mas o nome reflete seu comportamento.
// Retorno: 0 em caso de sucesso ou -1 c.c.
int move_recursivo (struct diretorio * diretorio, FILE * archive_pt, int pos, long deslocamento, int fim);

// Insere os dados, de tamanho tam, do membro, em pos, no archiver.
// Se a struct arquivo for passada, atualiza seu offset.
// Retorno: 0 em caso de sucesso e -1 c.c.
int insere_membro_arq (FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, struct arquivo * arquivo, unsigned long tam, int pos);

// Comprime um arquivo e muda campo de tam_comp da struct.
// Retorno: 0 em caso de sucesso e -1 c.c.
int comprime_arquivo (char * file_name, FILE * file_pt, struct arquivo * arquivo);

// Descomprime um arquivo e muda o campo de tam_comp para zero.
// Retorno: 0 em caso de sucesso e -1 c.c.
int descomprime_arquivo (char * file_name, FILE * file_pt, struct arquivo * arquivo);

// Deixa somente os bytes necessarios de um arquivo.vc.
void truncate_file (FILE * file_pt, struct diretorio * diretorio);

// Move um elemento do vetor, indicado por pos_1, para imediatamente depois de pos_2.
void move_elemento (struct diretorio * diretorio, int pos_1, int pos_2);

// Move um membro para o inicio do vetor, movendo o restante para o lado.
void move_inicio (struct diretorio * diretorio, int pos);

// Retira o elemento indicado e rearranja os outros corretamente.
void retira_elemento (struct diretorio * diretorio, int pos);

// Calcula e retorna a diferenca de tamanho de dois arquivos diferentes.
// Confere se algum dos dois esta compactado ou nao.
long diff_tam (struct arquivo * arq1, struct arquivo * arq2);

#endif