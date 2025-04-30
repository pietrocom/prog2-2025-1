#include "utils.h"

#include "vina.h"
#include "lz/lz.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
        for (int i = pos + 1; i < diretorio->qtd_membros; i++) {
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
    if (fseek(membro_pt, 0, SEEK_SET) != 0)
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

    if (fwrite(buffer, tam, 1, archive_pt) != 1)
        return -1;

    fflush(archive_pt);
    free(buffer);

    return 0;
}

int comprime_arquivo (char * file_name, FILE * file_pt, struct arquivo * arquivo) {
    if (!file_pt || !arquivo || !file_name)
        return -1;

    // Le o conteudo
    unsigned char * buffer_in = calloc(arquivo->tam_or, 1);
    if (!buffer_in)
        return -1;
    if (fseek(file_pt, 0, SEEK_SET) != 0)
        return -1;
    if (fread(buffer_in, arquivo->tam_or, 1, file_pt) != 1)
        return -1;

    // Buffer de saida 1% + 1 bytes maior que o de entrada (minimo exigido eh 0.4% + 1)
    unsigned char * buffer_out = calloc(arquivo->tam_or / 100 + arquivo->tam_or + 1, 1);
    if (!buffer_out) {
        free(buffer_in);
        return -1;
    }

    int tam_comp = LZ_Compress(buffer_in, buffer_out, arquivo->tam_or);

    // Atualiza tamanho do arquivo que esta compactado
    arquivo->tam_comp = tam_comp;

    // Escreve no arquivo o buffer_out
    file_pt = freopen(file_name, "wb", file_pt);

    if (!file_pt) {
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    fwrite(buffer_out, arquivo->tam_comp, 1, file_pt);

    // Reabre o arquivo no modo de leitura e edicao correto
    file_pt = freopen(file_name, "rb", file_pt);

    if (!file_pt) {
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    free(buffer_in);
    free(buffer_out);

    return 0;
}

void imprime_arquivo_info(struct arquivo *arq) {
    if (!arq) {
        printf("Arquivo NULL\n");
        return;
    }

    printf("Nome: %s\n", arq->nome);
    printf("UID: %d\n", arq->uid);
    printf("Tamanho original: %lu bytes\n", arq->tam_or);
    printf("Tamanho comprimido: %d bytes\n", arq->tam_comp);
    printf("Ordem: %d\n", arq->ordem);
    printf("Offset: %lu\n", arq->offset);
    
    // Formatação da data
    if (arq->mod_time != 0) {
        printf("Mod time: %s", ctime(&arq->mod_time)); 
    } else {
        printf("Mod time: (não definido)\n");
    }
    printf("-----------------\n");
}

void truncate_file (FILE * file_pt, struct diretorio * diretorio) {
    if (!file_pt || !diretorio)
        return;

    // Esvazia o buffer de saida
    fflush(file_pt);

    // Ultimo membro do diretorio
    struct arquivo * aux = diretorio->membros[diretorio->qtd_membros - 1];

    off_t len;
    if (aux->tam_comp == 0)
        len = (off_t)(aux->tam_or + aux->offset);
    else 
        len = (off_t)(aux->tam_comp + aux->offset);
    
    ftruncate(fileno(file_pt), len);

    // Reposiciona o ponteiro
    rewind(file_pt);
}

void troca_pos (struct diretorio * diretorio, int pos_1, int pos_2) {
    struct arquivo * aux = diretorio->membros[pos_1];
    diretorio->membros[pos_1] = diretorio->membros[pos_2];
    diretorio->membros[pos_2] = aux;
}

void move_inicio (struct diretorio * diretorio, int pos) {
    struct arquivo * aux = diretorio->membros[pos];
    for (int i = pos - 1; i <= 0; i--) 
        diretorio->membros[i + 1] = diretorio->membros[i];
        
    diretorio->membros[0] = aux;
}