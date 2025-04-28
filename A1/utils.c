#include "utils.h"

int move (unsigned long inicio, unsigned long fim, long deslocamento, FILE * file) {
    if (!file || inicio >= fim)
        return -1;

    // Tamanho do bloco a mover
    size_t tamanho = fim - inicio;
    char *buffer = malloc(tamanho);
    if (!buffer)
        return -1;

    // Le o bloco original
    fseek(file, inicio, SEEK_SET);
    fread(buffer, 1, tamanho, file);

    // Escreve na nova posicao
    fseek(file, inicio + deslocamento, SEEK_SET);
    fwrite(buffer, 1, tamanho, file);

    free(buffer);
    return 0;
}

int move_recursivo (struct diretorio * diretorio, FILE * archive_pt, int pos, long deslocamento) {
    if (!diretorio || !archive_pt)
        return -1;

    // Cuida para que nao haja sobrescrita de informacao
    if (deslocamento >= 0) {
        // Move, do fim ate pos, todos os membros a frente deslocamento bytes
        for (int i = diretorio->qtd_membros - 1; i >= pos; i--) {
            if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, deslocamento, archive_pt) == -1)
                return -1;
        }
    }
    else {
        // Move, de pos ate fim, todos os membros a frente deslocamento bytes
        for (int i = pos; i < diretorio->qtd_membros; i++) {
            if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, deslocamento, archive_pt) == -1)
                return -1;
        }
    }

    atualiza_metadados(diretorio);

    return 0;
}

int insere_membro_arq (FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, struct arquivo * arquivo, unsigned long tam, int pos) {
    if (!membro_pt || ! archive_pt || !diretorio)
        return -1;

    fflush(archive_pt);

    // Le o conteudo
    char * buffer = calloc(tam, 1);
    if (!buffer)
        return -1;
    if (fread(buffer, tam, 1, membro_pt) != 1)
        return -1;

    // Atualiza o ponteiro para a posicao correta
    if (pos == -1) {
        if (fseek(archive_pt, 0, SEEK_END) != 0)
            return -1;
    }
    else {
        if (fseek(archive_pt, (long int)diretorio->membros[pos]->offset, SEEK_SET) != 0)
            return -1;
    }

    // Atualiza o offset do arquivo
    if (arquivo)
        arquivo->offset = ftell(archive_pt);

    // Escreve o conteudo
    if (fwrite(buffer, tam, 1, archive_pt) != 1)
        return -1;

    fflush(archive_pt);
    free(buffer);

    return 0;
}