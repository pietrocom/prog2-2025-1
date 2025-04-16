#ifndef VINA_H
#define VINA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Defines ----

#define NOME_MAX 100
#define DATA_MAX 20


// ---- Structs ----

struct arquivo {
    char nome[NOME_MAX];    
    int uid;       
    int tam_or;          
    int tam_comp;         
    char data_mod[DATA_MAX]; 
    int ordem;              
    int offset;              
};


// ---- Funcoes ----



#endif