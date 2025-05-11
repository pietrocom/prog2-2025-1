#include "options.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lz/lz.h"
#include "aux.h"
#include "vina.h"
#include "utils.h"


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

    // Cria uma struct com os dados do novo arquivo, se nao foi passado
    struct arquivo * novo_arq = cria_s_arquivo();
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
    move_recursivo(diretorio, archive_pt, 0, sizeof(struct arquivo), -1);

    // Escreve no archiver a struct diretorio
    escreve_s_diretorio(diretorio, archive_pt);

    fclose(membro_pt);
    fclose(archive_pt);

    return 0;
}

int ic (struct diretorio * diretorio, char * membro, char * archive) {
    if (!membro || !archive || !diretorio)
        return -1;
    
    // Confere se o membro ja existe
    int pos;
    for (pos = 0; pos < diretorio->qtd_membros; pos++)
        if (strcmp(membro, diretorio->membros[pos]->nome) == 0)
            // Trata do caso de o membro ja existir
            return ic_existe(diretorio, membro, archive, pos);

    // Abre o membro
    FILE *membro_pt = fopen(membro, "rb");
    if (!membro_pt)
        return -1;
    
    // Cria uma struct com os dados do novo arquivo
    struct arquivo *novo_arq = cria_s_arquivo();
    if (!novo_arq) {
        fclose(membro_pt);
        return -1;
    }
    novo_arq = inicia_s_arquivo(novo_arq, membro);
    if (!novo_arq) {
        fclose(membro_pt);
        destroi_s_arquivo(novo_arq);
        return -1;
    }

    // Comprime o arquivo e atualiza metadados
    if (comprime_arquivo(membro, membro_pt, novo_arq) == -1)
        return -1;

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        fclose(membro_pt);
        return -1;
    }

    // Insere os dados do membro no archiver - Offset atualizado
    if (novo_arq->tam_comp == 0)
        insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_or, -1);
    else 
        insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_comp, -1);

    // Atualiza o vetor de ponteiros para struct arquivo - qtd_membros atualizada
    insere_s_arquivo(diretorio, novo_arq, -1);

    // Move todos os membros sizeof(struct arquivo) para frente - Offset atualizado
    move_recursivo(diretorio, archive_pt, 0, sizeof(struct arquivo), -1);

    // Escreve no archiver a struct diretorio
    escreve_s_diretorio(diretorio, archive_pt);

    fclose(membro_pt);
    fclose(archive_pt);

    return 0;
}

int m (struct diretorio * diretorio, char * membro, char * target, char * archive) {
    if (!diretorio || !membro || !archive)
        return -1;

    // Confere se o membro existe e extrai sua posicao se sim
    int pos_mem;
    for (pos_mem = 0; pos_mem < diretorio->qtd_membros; pos_mem++)
        if (strcmp(membro, diretorio->membros[pos_mem]->nome) == 0)
            break;
    if (pos_mem == diretorio->qtd_membros) {
        printf("Erro: membro nao existe!\n");
        return -1;
    }

    // Confere se o target existe e extrai sua posicao se sim
    int pos_tar;
    if (strcmp(target, "NULL") == 0)
        pos_tar = -1;
    else {
        for (pos_tar = 0; pos_tar < diretorio->qtd_membros; pos_tar++)
            if (strcmp(target, diretorio->membros[pos_tar]->nome) == 0)
                break;
        if (pos_tar == diretorio->qtd_membros) {
            printf("Erro: target nao existe!\n");
            return -1;
        }
    }

    if ( (pos_mem == pos_tar) || ((pos_mem == 0) && (pos_tar == -1)) ) {
        printf("Aviso: membro não movimentado!\n");
        return 0;
    }

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        return -1;
    }

    // Define structs auxiliares para operacoes
    struct arquivo * membro_s = diretorio->membros[pos_mem];
    struct arquivo * target_s; 
    if (pos_tar > -1)
        target_s = diretorio->membros[pos_tar];
    else 
        target_s = NULL;
    // Define os tamanhos corretos (compactado ou nao)
    unsigned long tam_mem;
    unsigned long tam_tar;
    if (membro_s->tam_comp == 0)
        tam_mem = membro_s->tam_or;
    else 
        tam_mem = membro_s->tam_comp;
    if (target_s) {
        if (target_s->tam_comp == 0)
            tam_tar = target_s->tam_or;
        else
            tam_tar = target_s->tam_comp;
    }
    else
        tam_tar = 0;

    // Aloca um buffer para o membro
    char * buffer;
    buffer = malloc(tam_mem);
    if (!buffer) {
        fclose(archive_pt);
        return -1;
    }

    // Le o conteudo do membro e armazena no buffer
    fseek(archive_pt, membro_s->offset, SEEK_SET);
    fread(buffer, tam_mem, 1, archive_pt);

    // Move todos os membros para dar espaço para o membro
    if (pos_tar >= pos_mem) {
        if (move_recursivo(diretorio, archive_pt, pos_mem, -tam_mem, pos_tar + 1) == -1)
            return -1;
    }
    else if (pos_tar == -1) {
        pos_tar++;
        if (move_recursivo(diretorio, archive_pt, pos_tar, tam_mem, pos_mem) == -1)
            return -1;
    }
    else
        if (move_recursivo(diretorio, archive_pt, pos_tar, tam_mem, pos_mem + 1) == -1)
            return -1;

    // Escreve o membro no seu lugar
    if (pos_tar > pos_mem)
        fseek(archive_pt, diretorio->membros[pos_tar]->offset + tam_tar - tam_mem, SEEK_SET);
    else
        fseek(archive_pt, diretorio->membros[pos_tar]->offset + tam_tar, SEEK_SET);
    fwrite(buffer, tam_mem, 1, archive_pt);

    // Move o elemento de pos_mem para imediatamente depois de pos_tar
    if (target_s)
        move_elemento(diretorio, pos_mem, pos_tar);
    else 
        move_inicio(diretorio, pos_mem);

    atualiza_metadados(diretorio);

    // Escreve no archiver a struct diretorio
    escreve_s_diretorio(diretorio, archive_pt);

    // Trunca o arquivo 
    truncate_file(archive_pt, diretorio);

    fclose(archive_pt);
    free(buffer);

    return 0;
}

int x (struct diretorio * diretorio, char * file_name, char * archive) {
    if (!diretorio || !archive)
        return -1;

    // Verifica se o membro existe e extrai a posicao se sim
    int pos_mem;
    for (pos_mem = 0; pos_mem < diretorio->qtd_membros; pos_mem++) {
        if (strcmp(file_name, diretorio->membros[pos_mem]->nome) == 0)
            break;
    }
    printf("qtd_membros = %d\n", diretorio->qtd_membros);
    if (pos_mem == diretorio->qtd_membros) {
        printf("Erro: membro nao existe!\n");
        return -1;
    }

    struct arquivo * file_s = diretorio->membros[pos_mem];
    unsigned long tam_mem;
    if (file_s->tam_comp == 0)
        tam_mem = file_s->tam_or;
    else 
        tam_mem = file_s->tam_comp;

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        return -1;
    }

    // Aloca um buffer para o membro
    char * buffer;
    buffer = malloc(tam_mem);
    if (!buffer) {
        fclose(archive_pt);
        return -1;
    }

    // Le o conteudo do membro e armazena no buffer
    fseek(archive_pt, file_s->offset, SEEK_SET);
    fread(buffer, tam_mem, 1, archive_pt);

    // Se o membro estiver na ponta do vetor
    if (pos_mem == diretorio->qtd_membros - 1) {
        move_recursivo(diretorio, archive_pt, -1, -sizeof(struct arquivo), diretorio->qtd_membros - 1);

        retira_elemento(diretorio, pos_mem);

        atualiza_metadados(diretorio);

        escreve_s_diretorio(diretorio, archive_pt);

        truncate_file(archive_pt, diretorio);
    }
    // c.c.
    else {
        move_recursivo(diretorio, archive_pt, pos_mem, -((long)tam_mem), diretorio->qtd_membros);

        move_recursivo(diretorio, archive_pt, -1, -(sizeof(struct arquivo)), -1);

        retira_elemento(diretorio, pos_mem);

        atualiza_metadados(diretorio);

        escreve_s_diretorio(diretorio, archive_pt);

        truncate_file(archive_pt, diretorio);
    }

    // Abre o novo arquivo para atualizacao
    FILE *novo_pt = fopen(file_name, "w+b");
    if (!novo_pt) {
        fclose(archive_pt);
        free(buffer);
        return -1;
    }

    // Escreve o conteudo nele
    fwrite(buffer, tam_mem, 1, novo_pt);

    free(buffer);

    // Descomprime se estiver comprimido
    if (file_s->tam_comp > 0)
        descomprime_arquivo(file_name, novo_pt, file_s);

    fclose(novo_pt);
    fclose(archive_pt);

    return 0;
}

int r (struct diretorio * diretorio, char * file_name, char * archive) {
    
}