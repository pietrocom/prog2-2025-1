#include "opt_aux.h"

int ip_mem_igual (struct diretorio * diretorio, char * membro, char * archive, int pos) {
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
    if (! (struct arquivo *novo_arq = cria_s_arquivo()) )
        return -1;
    novo_arq = inicia_valores_arquivo(novo_arq, membro);

    // Calcula a diferenca de tamanho dos arquivos existentes
    long dif_tam = diretorio->membros[pos]->tam_or - novo_arq->tam_or;

    // Move, do fim ao comeco, todos os membros a frente dif_tam bytes
    for (int i = diretorio->qtd_membros; i > pos; i--) {
        if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, dif_tam, archive_pt) == -1)
            return -1;
    }

    // Insere os dados do membro em pos no archiver
    

    // Atualiza os metadados dos arquivos
    destroi_s_arquivo(diretorio->membros[pos]);
    diretorio->membros[pos] = novo_arq;
    unsigned long offset = sizeof(int);
    for (int i = 0; i < diretorio->qtd_membros; i++) {
        diretorio->membros[i]->offset = offset;
        offset += diretorio->membros[i]->tam_or;
        diretorio->membros[i]->ordem = i;
    }

}