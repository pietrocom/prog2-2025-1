#include "options.h"

int ip (struct diretorio * diretorio, char * membro, char * archive) {
    if (!membro || !archive || !diretorio)
        return -1;

    // Confere se o membro ja existe
    for (int i = 0; i < diretorio->qtd_membros; i++)
        if (strcmp(membro, diretorio->membros[i]->nome) == 0)
            // Trata do caso de o membro ja existir
            return ip_mem_existe(diretorio, membro, archive, i);

    
    struct arquivo *novo_arq = cria_s_arquivo();
    if (!novo_arq)
        return -1;
    novo_arq = inicia_s_arquivo(novo_arq, membro);
    if (!novo_arq)
        return -1;

    // Abre arquivos para ler e escrever
    FILE *file_pt = fopen(membro, "rb");
    FILE *archiver_pt = fopen(archive, "r+b");
    if (!file_pt || !archiver_pt)
        return -1;

    // Insere no final do vetor
    if (insere_s_arquivo(diretorio, novo_arq, -1) == -1)
        return -1;

    // Aloca espaco para o buffer
    char *buffer = malloc(novo_arq->tam_or);
    if (!buffer)
        return -1;

    // Escreve o conteudo do arquivo no final do archiver
    insere_membro_arq(file_pt, archiver_pt, diretorio, novo_arq->tam_or, diretorio->qtd_membros - 1);

    // Move todos os membros sizeof struct arquivo para frente 
    move_recursivo(diretorio, archiver_pt, diretorio->qtd_membros * sizeof(struct arquivo) + sizeof(int), sizeof(struct arquivo));

    // Atualiza metadados dos arquivos 
    atualiza_metadados(diretorio);

    // Escreve no archiver a struct diretorio
    escreve_s_diretorio(diretorio, archiver_pt);

    fclose(file_pt);
    fclose(archiver_pt);
    free(buffer);

    return 0;
}