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


int ip (struct diretorio * diretorio, char * membro, char * archive, struct arquivo * novo_arq) {
    if (!membro || !archive || !diretorio)
        return -1;

    // Confere se o membro ja existe
    int pos;
    for (pos = 0; pos < diretorio->qtd_membros; pos++)
        if (strcmp(membro, diretorio->membros[pos]->nome) == 0)
            // Trata do caso de o membro ja existir
            return ip_existe(diretorio, membro, archive, pos, novo_arq);
         
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
    if (!novo_arq) {
        novo_arq = cria_s_arquivo();
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
    }

    // Insere os dados do membro no archiver - Offset atualizado
    if (novo_arq->tam_comp == 0)
        insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_or, -1);
    else 
        insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_comp, -1);

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

    fclose(membro_pt);

    if (ip(diretorio, membro, archive, novo_arq) == -1)
        return -1;

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
    for (pos_tar = 0; pos_tar < diretorio->qtd_membros; pos_tar++)
        if (strcmp(membro, diretorio->membros[pos_tar]->nome) == 0)
            break;
    if ( (pos_tar == diretorio->qtd_membros) && (target) ) {
        printf("Erro: membro nao existe!\n");
        return -1;
    }
    // Trata a possibilidade de ser NULL
    if (!target)
        pos_tar = 0;

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        return -1;
    }

    // Define structs auxiliares para operacoes
    struct arquivo * membro_s = diretorio->membros[pos_mem];
    struct arquivo * target_s = diretorio->membros[pos_tar];
    // Remaneja os membros para troca
    if (pos_mem > pos_tar) {
        membro_s = diretorio->membros[pos_tar];
        target_s = diretorio->membros[pos_mem];
    }

    // Encontra a diferenca de tamanho
    long dif_tam = diff_tam(membro_s, target_s);

    // Aloca um buffer para fazer a operacao
    char * buffer;
    if (target_s->tam_comp == 0)
        buffer = malloc(target_s->tam_or);
    else
        buffer = malloc(target_s->tam_comp);
    if (!buffer) {
        fclose(archive_pt);
        return -1;
    }

    // Le o conteudo do target e armazena no buffer
    fseek(archive_pt, target_s->offset, SEEK_SET);
    if (target_s->tam_comp == 0) 
        fread(buffer, target_s->tam_or, 1, archive_pt);
    else
        fread(buffer, target_s->tam_comp, 1, archive_pt);

    // Move o membro para o seu lugar
    if (target_s->tam_comp == 0)
        move(membro_s->offset, membro_s->offset + membro_s->tam_or, dif_tam, archive_pt);
    else 
        move(membro_s->offset, membro_s->offset + membro_s->tam_comp, dif_tam, archive_pt);

    // Move todos os membros entre membro e target dif_tam bytes
    if (move_recursivo(diretorio, archive_pt, pos_mem, dif_tam, pos_tar) == -1)
        return -1;

    // Escreve o conteudo de target na posicao de membro
    fseek(archive_pt, membro_s->offset, SEEK_SET);
    if (target_s->tam_comp == 0) 
        fwrite(buffer, target_s->tam_or, 1, archive_pt);
    else
        fwrite(buffer, target_s->tam_comp, 1, archive_pt);

    // Se for posicao 0
    if (!target)
        move_inicio (diretorio, pos_mem);
    else
        // Troca no vetor as posicoes das structs
        troca_pos (diretorio, pos_mem, pos_tar);

    atualiza_metadados(diretorio);

    fclose(archive_pt);
    free(buffer);

    return 0;
}