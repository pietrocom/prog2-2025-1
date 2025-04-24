#include "vina.h"
#include "utils.h"
#include "lz/lz.h"

struct arquivo * cria_s_arquivo () {
    struct arquivo * file = malloc(sizeof(struct arquivo));
    if (!file)
        return NULL;
    
    return file;
}

void destroi_s_arquivo (struct arquivo * file) {
    free(file);
}

struct diretorio * cria_diretorio () {
    struct diretorio * diretorio = malloc(sizeof(struct diretorio));
    if (!diretorio)
        return NULL;

    diretorio->capacidade = CAPACIDADE_DIR;
    diretorio->qtd_membros = 0;
    // Cria um vetor de ponteiros para arquivos
    diretorio->membros = malloc(sizeof(struct arquivo) * CAPACIDADE_DIR);

    return diretorio;
}

void destroi_diretorio (struct diretorio * diretorio) {
    fprio_destroi(diretorio->fila);
    free(diretorio);
}

void insere_arquivo (struct diretorio * diretorio, struct arquivo * arq, FILE * file) {
    if (!diretorio || !file)
        return;

    
}