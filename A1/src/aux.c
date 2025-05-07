#include "aux.h"

#include "utils.h"
#include "vina.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int ip_existe (struct diretorio * diretorio, char * membro, char * archive, int pos, struct arquivo * novo_arq) {
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

    // Cria uma struct com os dados do novo arquivo, caso nao tenha sido passado
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

    // Calcula a diferenca de tamanho dos arquivos existentes
    long dif_tam = diff_tam(diretorio->membros[pos], novo_arq);
    
    // Move todos os membros a frente de pos dif_tam bytes para frente 
    move_recursivo(diretorio, archive_pt, pos, dif_tam, -1);

    // Insere os dados do membro no archiver
    if (novo_arq->tam_comp == 0)
        insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_or, pos);
    else 
        insere_membro_arq(membro_pt, archive_pt, diretorio, novo_arq, novo_arq->tam_comp, pos);

    // Libera o arquivo antigo antes de substituir
    destroi_s_arquivo(diretorio->membros[pos]);
    
    // Atualiza o vetor de ponteiros para struct arquivo
    diretorio->membros[pos] = novo_arq;

    // Atualiza metadados dos arquivos 
    atualiza_metadados(diretorio);

    // Escreve no archiver a struct diretorio
    escreve_s_diretorio(diretorio, archive_pt);

    // Trunca o archive
    truncate_file(archive_pt, diretorio);

    fclose(membro_pt);
    fclose(archive_pt);

    return 0;
}