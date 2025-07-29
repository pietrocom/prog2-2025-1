#include "vina.h"
#include "utils.h"
#include "options.h"
#include "aux.h"
#include <string.h>
#include <stdlib.h>


int main (int argc, char *argv[]) {

    char *opcao = argv[1];
    char *archive = argv[2];

    struct diretorio * diretorio = cria_diretorio();
    if (!diretorio)
        return -1;

    // Diretorio preenchido com metadados dos arquivos
    if (inicia_diretorio(diretorio, archive) == -1) {
        destroi_diretorio(diretorio);
        return -1;
    }

    if ( (strcmp(opcao, "-ip") == 0) || (strcmp(opcao, "-p") == 0) ) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            if (ip(diretorio, membro, archive) == -1) {
                destroi_diretorio(diretorio);
                return -1;
            }
        }
    }

    else if ( (strcmp(opcao, "-ic") == 0) || (strcmp(opcao, "-i") == 0) ) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            if (ic(diretorio, membro, archive) == -1) {
                destroi_diretorio(diretorio);
                return -1;
            }
        }
    }

    else if (strcmp(opcao, "-m") == 0) {
        if (argc != 5) {
            printf("Argumentos invalidos!\n");
            destroi_diretorio(diretorio);
            return -1;
        }
        char *membro = argv[3];
        char *target = argv[4];
        if (m(diretorio, membro, target, archive) == -1) {
            destroi_diretorio(diretorio);
            return -1;
        }
    }

    else if (strcmp(opcao, "-x") == 0) {
        if (argc == 3) {
            for (int i = diretorio->qtd_membros - 1; i >= 0; i--) {
                if (x(diretorio, diretorio->membros[i]->nome, archive) == -1) {
                    destroi_diretorio(diretorio);
                    return -1;
                }
            }
        }
        else if (argc > 3) {
            for (int i = 3; i < argc; i++) {
                char *membro = argv[i];
                if (x(diretorio, membro, archive) == -1) {
                    destroi_diretorio(diretorio);
                    return -1;
                }
            }
        }
    }

    else if (strcmp(opcao, "-r") == 0) {
        if (argc > 3) {
            for (int i = 3; i < argc; i++) {
                char *membro = argv[i];
                if (r(diretorio, membro, archive) == -1) {
                    destroi_diretorio(diretorio);
                    return -1;
                }
            }
        }
        else if (argc == 3) {
            for (int i = diretorio->qtd_membros -1; i >= 0; i--) {
                if (r(diretorio, diretorio->membros[i]->nome, archive) == -1) {
                    destroi_diretorio(diretorio);
                    return -1;
                }
            }
        }
    }

    else if (strcmp(opcao, "-c") == 0) {
        if (argc > 3) {
            printf("Argumentos invalidos!");
            destroi_diretorio(diretorio);
            return -1;
        }
        c(diretorio);
    }

    else if (strcmp(opcao, "-z") == 0) {
        // Abre o archive se existir
        FILE *archive_pt = fopen(archive, "rb");
        if (!archive_pt) {
            printf("Archive .vc inexistente!\n");
            return -1;
        }

        struct diretorio * novo_diretorio = cria_diretorio();
        if (!novo_diretorio)
            return -1;

        // Adiciona _z no final do nome
        int tam_nome_archive = strlen(archive) - 3;
        char *novo_nome;
        novo_nome = calloc(tam_nome_archive + 5, 1);
        memcpy(novo_nome, archive, tam_nome_archive);
        strcat(novo_nome, "_z.vc");

        FILE *novo_archive_pt = fopen(novo_nome, "w+b");
        if (!novo_archive_pt) {
            printf("ponteiro nulo\n");
            return -1;
        }

        // Diretorio preenchido com metadados dos arquivos
        if (inicia_diretorio(novo_diretorio, novo_nome) == -1) {
            //destroi_diretorio(novo_diretorio);
            return -1;
        }

        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            if (z(diretorio, novo_diretorio, membro, archive_pt, novo_archive_pt) == -1) {
                destroi_diretorio(diretorio);
                //destroi_diretorio(novo_diretorio);
                fclose(novo_archive_pt);
                fclose(archive_pt);
                return -1;
            }
        }

        fclose(novo_archive_pt);
        fclose(archive_pt);
        //destroi_diretorio(novo_diretorio);
        free(novo_nome);
    }   

    else {
        printf("Comando inexistente!\n");
    }

    destroi_diretorio(diretorio);

    return 0;
}

