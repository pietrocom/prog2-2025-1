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
    if (inicia_diretorio(diretorio, archive) == -1)
        return -1;

    if ( (strcmp(opcao, "-ip") == 0) || (strcmp(opcao, "-p") == 0) ) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            if (ip(diretorio, membro, archive) == -1)
                return -1;
        }
    }

    else if ( (strcmp(opcao, "-ic") == 0) || (strcmp(opcao, "-i") == 0) ) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            if (ic(diretorio, membro, archive) == -1)
                return -1;
        }
    }

    else if (strcmp(opcao, "-m") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
        }
    }

    else if (strcmp(opcao, "-x") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
        }
    }

    else if (strcmp(opcao, "-r") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
        }
    }

    else if (strcmp(opcao, "-c") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
        }
    }

    else {
        // erro na escrita do comando
    }

    destroi_diretorio(diretorio);

    return 0;
}

