#include "utils.h"

struct arquivo * inicia_valores_arquivo (struct arquivo * arquivo, char * nome) {
    if (!arquivo || !nome)
        return NULL;

    // Preenche os dados basicos do arquivo
    strncpy(arquivo->nome, nome, NOME_MAX);
    
    // Obtem informacoes do arquivo no sistema de arquivos
    struct stat st;
    if (stat(nome, &st) != 0)
        return NULL;

    arquivo->uid = st.st_uid;
    arquivo->tam_or = st.st_size;
    arquivo->tam_comp = st.st_size; // Sem compressao inicialmente
    arquivo->ordem = -1; // Sera definido na insercao
    arquivo->offset = 0; // Sera calculado depois

    return arquivo;
}

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
    if (fread(buffer, 1, tamanho, file) != tamanho) {
        free(buffer);
        return -1;
    }

    // Escreve na nova posicao
    fseek(file, inicio + deslocamento, SEEK_SET);
    if (fwrite(buffer, 1, tamanho, file) != tamanho) {
        free(buffer);
        return -1;
    }

    free(buffer);
    return 0;
}


int insere_s_arquivo (struct diretorio * diretorio, struct arquivo * arquivo, int pos) {
    if (!diretorio || !arquivo)
        return -1;

    // Se posicao for -1, insere no final
    if (pos == -1)
        pos = diretorio->qtd_membros;

    // Realoca o vetor de membros
    struct arquivo **novo = realloc(diretorio->membros, (diretorio->qtd_membros + 1) * sizeof(struct arquivo *));
    if (!novo)
        return -1;

    diretorio->membros = novo;

    // Desloca os elementos para abrir espaco
    for (int i = diretorio->qtd_membros; i > pos; i--) {
        diretorio->membros[i] = diretorio->membros[i-1];
        diretorio->membros[i]->ordem = i; // Atualiza a ordem
    }

    // Insere o novo arquivo e atualiza contador
    diretorio->membros[pos] = arquivo;
    arquivo->ordem = pos;
    diretorio->qtd_membros++;

    return 0;
}