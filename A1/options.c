#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

void imprime_membro(FILE *archiver, struct arquivo *arquivo) {
    if (!archiver || !arquivo) {
        printf("Erro: ponteiros inválidos\n");
        return;
    }

    printf("\n=== DEBUG: Conteúdo do arquivo '%s' ===\n", arquivo->nome);
    printf("Offset: %lu, Tamanho: %lu bytes\n", arquivo->offset, arquivo->tam_or);

    // Salva a posição atual no arquivo
    long posicao_atual = ftell(archiver);
    if (posicao_atual == -1) {
        perror("Erro ao obter posição atual no arquivo");
        return;
    }

    // Posiciona no início do conteúdo do arquivo
    if (fseek(archiver, arquivo->offset, SEEK_SET) != 0) {
        perror("Erro ao posicionar no arquivo");
        return;
    }

    // Aloca buffer para leitura
    char *buffer = malloc(arquivo->tam_or + 1);
    if (!buffer) {
        printf("Erro ao alocar memória\n");
        return;
    }

    // Lê o conteúdo
    size_t lidos = fread(buffer, 1, arquivo->tam_or, archiver);
    if (lidos != arquivo->tam_or) {
        printf("Aviso: leu %zu bytes, esperava %lu\n", lidos, arquivo->tam_or);
    }

    // Garante terminação nula para impressão segura
    buffer[lidos] = '\0';

    // Imprime o conteúdo
    printf("[Conteúdo]\n%.*s\n", (int)lidos, buffer);
    printf("[Bytes hex]\n");
    for (size_t i = 0; i < lidos; i++) {
        printf("%02x ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    // Restaura a posição original
    fseek(archiver, posicao_atual, SEEK_SET);

    free(buffer);
    printf("=== Fim do debug ===\n\n");
}

int ip (struct diretorio * diretorio, char * membro, char * archive) {
    if (!membro || !archive || !diretorio)
        return -1;

    // Confere se o membro ja existe
    int pos;
    for (pos = 0; pos < diretorio->qtd_membros; pos++)
        if (strcmp(membro, diretorio->membros[pos]->nome) == 0)
            // Trata do caso de o membro ja existir
            return ip_existe(diretorio, membro, archive, pos);
         
    // Abre o membro
    FILE *membro_pt = fopen(membro, "rb");
    if (!membro_pt)
        return -1;

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        fclose(membro_pt);
        return -1;
    }

    // Cria uma struct com os dados do novo arquivo
    struct arquivo *novo_arq = cria_s_arquivo();
    if (!novo_arq) {
        fclose(membro_pt);
        fclose(archive_pt);
        return -1;
    }
    
    novo_arq = inicia_s_arquivo(novo_arq, membro);
    if (!novo_arq) {
        fclose(membro_pt);
        fclose(archive_pt);
        destroi_s_arquivo(novo_arq);
        return -1;
    }

    // Insere os dados do membro no archiver - Offset atualizado
    insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_or, -1);
    
    // Atualiza o vetor de ponteiros para struct arquivo - qtd_membros atualizada
    insere_s_arquivo(diretorio, novo_arq, pos);

    // Move todos os membros sizeof(struct arquivo) para frente - Offset atualizado
    move_recursivo(diretorio, archive_pt, 0, sizeof(struct arquivo));

    // Escreve no archiver a struct diretorio
    escreve_s_diretorio(diretorio, archive_pt);

    fclose(membro_pt);
    fclose(archive_pt);

    return 0;
}