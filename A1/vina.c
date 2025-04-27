#include "vina.h"
#include "utils.h"
#include "lz/lz.h"

FILE * cria_arquivo (char * file) {
    FILE * file_pt = fopen(file, "wb");
    if (!file_pt)
        return NULL;
    
    int qtd_membros = 0;
    // Inicia o primeiro int do arquivo (qtd_membros) com 0
    if ( fwrite(&qtd_membros, sizeof(int), 1, file_pt) != 1 ) {
        fclose(file_pt);
        return NULL; // Erro na escrita
    }

    return file_pt;
}

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
    if (!diretorio)
        return;

    for (int i = 0; i < diretorio->qtd_membros; i++) {
        destroi_s_arquivo(diretorio->membros[i]);
    }

    free(diretorio->membros);
    free(diretorio);
}

int inicia_diretorio (struct diretorio * diretorio, char * file) {
    if (!diretorio || !file)
        return -1;
    
    int qtd_membros = 0;
    FILE * file_pt = fopen(file, "rb");

    if (!file_pt) {
        file_pt = cria_arquivo(file);
        if (!file_pt)
            return -1;
    }

    // Ler o primeiro inteiro do arquivo
    fread(&qtd_membros, sizeof(int), 1, file_pt);
    diretorio->qtd_membros = qtd_membros;

    // Se nao houver membros no diretorio retorna
    if (qtd_membros == 0) {
        fclose(file_pt);
        return 0;
    }

    // Caso hajam membros, serao inicializados no vetor da struct diretorio
    diretorio->membros = malloc(sizeof(struct arquivo *) * qtd_membros);
    if (!diretorio->membros)
        return -1;
    
    for (int i = 0; i < qtd_membros; i++) {
        // Aloca espaco para o arquivo 
        if ( !(diretorio->membros[i] = cria_s_arquivo()) )
            return -1;

        // Ponteiro para o inicio da struct de interesse
        fseek(file_pt, sizeof(struct arquivo) * i + sizeof(int), SEEK_SET);
        // Cada ponteiro aponta para sua respectiva struct arquivo 
        fread(diretorio->membros[i], sizeof(struct arquivo), 1, file_pt);
    }

    fclose(file_pt);

    return 0;
}