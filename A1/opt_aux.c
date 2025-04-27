#include "opt_aux.h"

int insere_membro_arq(FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, unsigned long tam, int pos) {
    if (!membro_pt || ! archive_pt || !diretorio)
        return -1;

    char * buffer = malloc(tam);
    if (fread(buffer, tam, 1, membro_pt) != 0)
        return -1;
    if (fseek(membro_pt, (long int)diretorio->membros[pos]->offset, SEEK_SET) != 0)
        return -1;
    if (fwrite(buffer, tam, 1, archive_pt) != 0)
        return -1;

    free(buffer);

    return 0;
}

int ip_mem_existe (struct diretorio * diretorio, char * membro, char * archive, int pos) {
    if (!diretorio || !membro || !archive)
        return -1;
     
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
    if (!novo_arq)
        return -1;
    novo_arq = inicia_valores_arquivo(novo_arq, membro);

    // Calcula a diferenca de tamanho dos arquivos existentes
    long dif_tam = diretorio->membros[pos]->tam_or - novo_arq->tam_or;
    
    if (dif_tam >= 0) {
        // Move, do fim ate pos, todos os membros a frente dif_tam bytes
        for (int i = diretorio->qtd_membros; i > pos; i--) 
            if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, dif_tam, archive_pt) == -1)
                return -1;
    }
    else {
        // Move, de pos ate o fim, todos os membros a frente de dif_tam bytes
        for (int i = pos; i < diretorio->qtd_membros; i++) 
            if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, dif_tam, archive_pt) == -1)
                return -1;
    }

    // Insere os dados do membro no archiver
    insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq->tam_or, pos);

    // Atualiza os metadados dos arquivos
    destroi_s_arquivo(diretorio->membros[pos]);
    diretorio->membros[pos] = novo_arq;
    unsigned long offset = sizeof(int);
    for (int i = 0; i < diretorio->qtd_membros; i++) {
        diretorio->membros[i]->offset = offset;
        offset += diretorio->membros[i]->tam_or;
        diretorio->membros[i]->ordem = i;
    }

    fclose(membro_pt);
    fclose(archive_pt);

    return 0;
}