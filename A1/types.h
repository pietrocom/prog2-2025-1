#ifndef TYPES_H
#define TYPES_H


// ---- Defines ----

#define NOME_MAX 512


// ---- Structs ----

struct arquivo {
    char nome[NOME_MAX];    
    int uid;       
    unsigned long tam_or;          
    unsigned long tam_comp;         
    int ordem; 
    unsigned long offset;       
};

struct diretorio {
    int qtd_membros;
    // Vetor de ponteiros para structs no diretorio
    struct arquivo **membros;
};


#endif