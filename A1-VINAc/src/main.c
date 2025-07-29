#include "vina.h"
#include "utils.h"
#include "options.h"
#include "aux.h"
#include <string.h>


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

    else {
        printf("Comando inexistente!\n");
    }

    destroi_diretorio(diretorio);

    return 0;
}

