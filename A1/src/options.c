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
    int pos_tar = -1;
    if (target) {
        for (pos_tar = 0; pos_tar < diretorio->qtd_membros; pos_tar++)
            if (strcmp(target, diretorio->membros[pos_tar]->nome) == 0)
                break;
        if (pos_tar == diretorio->qtd_membros) {
            printf("Erro: target nao existe!\n");
            return -1;
        }
    }

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        return -1;
    }

    // Define structs auxiliares para operacoes
    struct arquivo * membro_s = diretorio->membros[pos_mem];
    struct arquivo * target_s; 
    if (target)
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
    if (target) {
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
        if (move_recursivo(diretorio, archive_pt, pos_mem, -tam_mem, pos_tar) == -1)
            return -1;
    }
    else
        if (move_recursivo(diretorio, archive_pt, pos_mem, tam_mem, pos_tar) == -1)
            return -1;

    // Escreve o membro no seu lugar
    if (target) {
        if (pos_tar > pos_mem)
            fseek(archive_pt, diretorio->membros[pos_tar]->offset + tam_tar - tam_mem, SEEK_SET);
        else
            fseek(archive_pt, diretorio->membros[pos_tar]->offset + tam_tar, SEEK_SET);
    }
    else
        fseek(archive_pt, sizeof(struct arquivo) * diretorio->qtd_membros, SEEK_SET);
    fwrite(buffer, tam_mem, 1, archive_pt);

    // Move o elemento de pos_mem para imediatamente depois de pos_tar
    if (!target)
        move_inicio(diretorio, pos_mem);
    else 
        move_elemento(diretorio, pos_mem, pos_tar);

    atualiza_metadados(diretorio);

    fclose(archive_pt);
    free(buffer);

    return 0;
}