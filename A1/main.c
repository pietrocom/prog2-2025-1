#include "vina.h"
#include "utils.h"

int main (int argc, char *argv[]) {
    
    char *opcao = argv[1];
    char *archive = argv[2];

    if (strcmp(opcao, "-ip") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
        }
    }

    else if (strcmp(opcao, "-ic") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
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

    return 0;
}
