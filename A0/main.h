#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>

// Structs

struct filajornal {
	struct noticia *prim;
	struct noticia *ult;
	unsigned int tam;
};

struct noticia {
	char titulo[33];
	char texto[513];
	char idade;	
	char prio;
	struct noticia *prox; 
};



// Funcoes

void requisita (char *titulo, char *texto);

// Cria e retorna a fila
struct filajornal *filacria ();

// Insere o item no final da fila
void filainsere (struct filajornal *fila, struct noticia *noticia);

// Remove o proximo da fila o retorna, ou NULL em erro 
// Nao da free no elemento retornado!
struct noticia *filaremove (struct filajornal *fila);

// Destroi os elementos da fila bem como a fila
void filadestroi (struct filajornal *fila);

// Cria uma noticia e a retorna, ou NULL em erro
struct noticia *crianoticia ();

// Adiciona mais um dia nas noticias da fila, removendo as 
// de mais de 3 dias
void filaatualiza (struct filajornal *fila);

// Cria e insere a noticia na fila correta
void cadastranoticia (struct filajornal *fila0, struct filajornal *fila1);

// Seleciona a noticia de maior prioridade e a retorna, ou NULL
// caso nao haja noticia para a edicao
struct noticia *selecionanoticia (struct filajornal *fila0, struct filajornal *fila1);

// Imprime a noticia caso exista
void imprimenoticia (struct noticia *noticia);

// Logica para imprimir a edicao
void imprimeedicao (struct filajornal *fila0, struct filajornal *fila1);

// Imprime o menu inicial
void imprimeopcoes ();

// Desaloca as listas 
void encerrasimulacao (struct filajornal *fila0, struct filajornal *fila1);


#endif
