#include "options.h"

int ip (struct diretorio * diretorio, char * membro, char * archive) {
    if (!membro || !archive || !diretorio)
        return -1;

    // Confere se o membro ja existe
    for (int i = 0; i < diretorio->qtd_membros; i++)
        if (strcmp(membro, diretorio->membros[i]->nome) == 0)
            // Trata do caso de o membro ja existir
            return ip_mem_existe(diretorio, membro, archive, i);

    // Se nao, insere no final do vetor e do arquivo
    struct arquivo *novo_arq = cria_s_arquivo();
    if (!novo_arq)
        return -1;
    novo_arq = inicia_valores_arquivo(novo_arq, membro);
    if (!novo_arq)
        return -1;

    // Insere no final do vetor
    if (insere_s_arquivo(diretorio, novo_arq, -1) == -1)
        return -1;

    // Insere o conteudo do membro no archiver
    char *buffer = malloc(novo_arq->tam_or);
    if (!buffer)
        return -1;

    FILE *file_pt = fopen(membro, "rb");
    FILE *archiver_pt = fopen(archive, "r+b");

    if (!file_pt || !archiver_pt)
        return -1;

    fseek(archiver_pt, 0, SEEK_END);             // aponta para o fim do archiver
    novo_arq->offset = ftell(archiver_pt);       // offset atualizado
    
    fread(buffer, novo_arq->tam_or, 1, file_pt);
    fwrite(buffer, novo_arq->tam_or, 1, archiver_pt);

    fclose(file_pt);
    fclose(archiver_pt);

    return 0;
}