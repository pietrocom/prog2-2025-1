#include "vina.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

FILE * cria_arquivo (char * file) {
    FILE * file_pt = fopen(file, "a+b");
    if (!file_pt)
        return NULL;
    
    unsigned long tamanho = ftell(file_pt);

    int qtd_membros = 0;
    // Inicia o primeiro int do arquivo (qtd_membros) com 0
    if (tamanho == 0) 
        fwrite(&qtd_membros, sizeof(int), 1, file_pt);

    return file_pt;
}

struct arquivo * cria_s_arquivo () {
    struct arquivo * file = malloc(sizeof(struct arquivo));
    if (!file)
        return NULL;
    
    file->uid = -1;
    file->ordem = -1;

    return file;
}

struct arquivo * inicia_s_arquivo (struct arquivo * arquivo, char * nome) {
    if (!arquivo || !nome)
        return NULL;

    // Preenche os dados basicos do arquivo
    strncpy(arquivo->nome, nome, NOME_MAX);
    arquivo->nome[NOME_MAX - 1] = '\0';
    
    // Obtem informacoes do arquivo no sistema de arquivos
    struct stat st;
    if (stat(nome, &st) != 0)
        return NULL;

        arquivo->uid = st.st_uid;
        arquivo->tam_or = (unsigned long)st.st_size;
        arquivo->tam_comp = 0;
        arquivo->ordem = -1;                    // Sera definido nas modificacoes
        arquivo->offset = 0;                    // Sera definido nas modificacoes
        arquivo->mod_time = st.st_mtime;

    return arquivo;
}

int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arquivo, int pos) {
    if (!diretorio || !arquivo)
        return -1;

    // Se posicao for -1, insere no final
    if (pos == -1)
        pos = diretorio->qtd_membros;

    // Realoca o vetor de membros
    struct arquivo **novo = realloc(diretorio->membros, (diretorio->qtd_membros + 1) * sizeof(struct arquivo *));
    if (!novo)
        return -1;

    diretorio->membros = novo;

    // Desloca os elementos para abrir espaco
    for (int i = diretorio->qtd_membros; i > pos; i--) {
        diretorio->membros[i] = diretorio->membros[i-1];
        diretorio->membros[i]->ordem = i; // Atualiza a ordem
    }

    // Insere o novo arquivo e atualiza contador
    diretorio->membros[pos] = arquivo;
    arquivo->ordem = pos;
    diretorio->qtd_membros++;

    return 0;
}

void atualiza_metadados (struct diretorio * diretorio) {
    unsigned long offset = sizeof(int) + sizeof(struct arquivo) * diretorio->qtd_membros;

    for (int i = 0; i < diretorio->qtd_membros; i++) {
        diretorio->membros[i]->offset = offset;
        offset += diretorio->membros[i]->tam_or;
        diretorio->membros[i]->ordem = i;
    }
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
    fseek(file_pt, 0, SEEK_SET);
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
        // Garantia de seguranca
        diretorio->membros[i]->nome[NOME_MAX - 1] = '\0';
    }

    fclose(file_pt);

    return 0;
}

int escreve_s_diretorio (struct diretorio * diretorio, FILE * archive_pt) {
    if (!diretorio || !archive_pt)
        return -1;

    // Posiciona o ponteiro no inicio do arquivo
    rewind(archive_pt);
    if (fwrite(&diretorio->qtd_membros, sizeof(int), 1, archive_pt) != 1)
        return -1;

    for (int i = 0; i < diretorio->qtd_membros; i++) {
        fwrite(diretorio->membros[i], sizeof(struct arquivo), 1, archive_pt);
    }

    return 0;
}