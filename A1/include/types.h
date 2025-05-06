#ifndef TYPES_H
#define TYPES_H


// ---- Defines ----

#define NOME_MAX 1024


// ---- Includes ----

#include <time.h>


// ---- Structs ----

struct arquivo {
    char nome[NOME_MAX];    
    int uid;       
    unsigned long tam_or;          
    int tam_comp;         
    int ordem; 
    unsigned long offset;
    time_t mod_time;
};

struct diretorio {
    int qtd_membros;
    // Vetor de ponteiros para structs no diretorio
    struct arquivo **membros;
};


#endif