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
        for (int i = diretorio->qtd_membros - 1; i > pos; i--) {
            if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, deslocamento, archive_pt) == -1)
                return -1;
            diretorio->membros[i]->offset += deslocamento;
        }
    }
    else {
        // Move, de pos ate o fim, todos os membros a frente deslocamento bytes
        for (int i = pos; i < diretorio->qtd_membros; i++) {
            if (move(diretorio->membros[i]->offset, diretorio->membros[i]->offset + diretorio->membros[i]->tam_or, deslocamento, archive_pt) == -1)
                return -1;
            diretorio->membros[i]->offset += deslocamento;
        }
    }

    return 0;
}

int insere_membro_arq (FILE * membro_pt, FILE * archive_pt, struct diretorio * diretorio, unsigned long tam, int pos) {
    if (!membro_pt || ! archive_pt || !diretorio)
        return -1;

    char * buffer = malloc(tam);
    if (fread(buffer, tam, 1, membro_pt) != 1)
        return -1;
    if (fseek(archive_pt, (long int)diretorio->membros[pos]->offset, SEEK_SET) != 0)
        return -1;
    if (fwrite(buffer, tam, 1, archive_pt) != 1)
        return -1;

    free(buffer);

    return 0;
}