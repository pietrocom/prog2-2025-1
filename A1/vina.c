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

    diretorio->qtd_membros = 0;
    // Vetor inicializado como vazio
    diretorio->membros = NULL;

    return diretorio;
}

void destroi_diretorio (struct diretorio * diretorio) {
    // Desalocar cada struct arquivo do vetor
    free(diretorio);
}

int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arq, FILE * file) {
    if (!diretorio || !file || !arq)
        return -1;

    // Aloca espaco para o novo membro
    diretorio->membros[diretorio->qtd_membros] = malloc(sizeof(struct arquivo))
    if (!diretorio->membros[diretorio->qtd_membros])
        return -1;
    
    diretorio->membros[diretorio->qtd_membros] = arq;

    diretorio->qtd_membros++;

    return 0;
}