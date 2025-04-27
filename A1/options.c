#include "options.h"

int ip (struct diretorio * diretorio, char * membro, char * archive) {
    if (!membro || !archive || !diretorio)
        return -1;

    // Confere se o membro ja existe
    for (int i = 0; i < diretorio->qtd_membros; i++)
        if (strcmp(membro, diretorio->membros[i]->nome) == 0)
            // Trata do caso de o membro ja existir
            return ip_mem_existe(struct diretorio * diretorio, char * membro, char * archive, i);

    // Se nao, insere no final do vetor e do arquivo
    if (insere_s_arquivo(diretorio, cria_s_arquivo(), -1) == -1)
        return -1;
    
    
}