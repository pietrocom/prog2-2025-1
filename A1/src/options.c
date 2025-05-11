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

    // Refazer a lógica da compressão!

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

int x (struct diretorio * diretorio, char * membro, char * archive) {
    if (!diretorio || !archive)
        return -1;

    int pos_mem;

    // Extrai todos os membros
    if (!membro) {
        pos_mem = -1;
    }

    // Confere se o membro existe e extrai sua posicao se sim
    else {
        for (pos_mem = 0; pos_mem < diretorio->qtd_membros; pos_mem++)
            if (strcmp(membro, diretorio->membros[pos_mem]->nome) == 0)
                break;
        if (pos_mem == diretorio->qtd_membros) {
            printf("Erro: membro nao existe!\n");
            return -1;
        }
    }

    // Abre o archiver para atualizacao
    FILE *archive_pt = fopen(archive, "r+b");
    if (!archive_pt) {
        return -1;
    }

    // Logica para remocao de um ou todos
    int inicio;
    int fim;
    if (pos_mem != -1) {
        inicio = pos_mem;
        fim = pos_mem + 1;
    }
    else {
        inicio = 0;
        fim = diretorio->qtd_membros;
    }
    
    // Loop para remocao do(s) membro(s)
    for (int i = inicio; i < fim; i++) {
        // Usar-se-a uma struct auxiliar
        struct arquivo * membro_s = diretorio->membros[i];

        // Define um tamanho para caso o membro esteja comprimido ou nao
        unsigned long tam;
        if (diretorio->membros[i]->tam_comp == 0)
            tam = diretorio->membros[i]->tam_or;
        else 
            tam = diretorio->membros[i]->tam_comp;

        // Aloca um buffer para operar sobre o membro
        char * buffer = calloc(tam, 1);
        if (!buffer) {
            fclose(archive_pt);
            return -1;
        }

        // Le o conteudo do membro e armazena no buffer
        fseek(archive_pt, membro_s->offset, SEEK_SET);
        fread(buffer, tam, 1, archive_pt);

        // Move os arquivos para frente do membro em questao para tras
        move_recursivo(diretorio, archive_pt, i, -tam, diretorio->qtd_membros);

        // Retira o membro do vetor de structs 
        retira_elemento(diretorio, pos_mem);

        // Move todos os membros sizeof(struct arquivo) para tras
        move_recursivo(diretorio, archive_pt, 0, -(sizeof(struct arquivo)), diretorio->qtd_membros);

        // Atualiza os offsets
        atualiza_metadados(diretorio);

        // Trunca o archive
        truncate_file(archive_pt, diretorio);

        free(buffer);
    }

    fclose(archive_pt);

    return 0;
}