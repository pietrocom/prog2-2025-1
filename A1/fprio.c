#include <stdio.h>
#include <stdlib.h>
#include "fprio.h"

struct fprio_t *fprio_cria () {
	struct fprio_t *nova;
	nova = malloc(sizeof(struct fprio_t));

	if (!nova)
		return NULL;

	nova->prim = NULL;
	nova->num = 0;
	return nova;
}

struct fprio_t *fprio_destroi (struct fprio_t *f) {
	if (!f)
		return NULL;

	struct fpnodo_t *aux = f->prim;

	while (aux != NULL) {
		f->prim = aux->prox;
		if (aux->item != NULL)
			free(aux->item);  
		free(aux);  
	aux = f->prim;
	}

	free(f);  
	f = NULL;  
	return NULL;  
}


// Retorna 1 caso os itens sejam iguais
int item_repetido (void *item_1, void *item_2) {
	if (item_1 == item_2)
		return 1;
	return 0;
}

int fprio_insere(struct fprio_t *f, void *item, int tipo, int prio) {
	if ( (!f) || (item == NULL) )
		return -1;

	struct fpnodo_t *aux = f->prim;

	struct fpnodo_t *novo = malloc(sizeof(struct fpnodo_t));
	if (!novo)
		return -1;

	novo->item = item;
	novo->tipo = tipo;
	novo->prio = prio;
	novo->prox = NULL;

	if (f->prim == NULL || prio < f->prim->prio) {
		novo->prox = f->prim;
		f->prim = novo;
		return f->num++;  
	}
	
	aux = f->prim;

	while (aux->prox != NULL && aux->prox->prio <= prio)
		aux = aux->prox;
	
	novo->prox = aux->prox;
	aux->prox = novo;

	return f->num++;  
}


void *fprio_retira (struct fprio_t *f, int *tipo, int *prio) {
	if ( (f == NULL) || (f->prim == NULL) || (tipo == NULL) || (prio == NULL) )  
		return NULL;
	
	struct fpnodo_t *aux = f->prim;
	void *item = aux->item;

	// Atualiza o primeiro nó para o próximo
	f->prim = aux->prox;
	f->num--;

	// Atribui valores somente se tipo e prio forem ponteiros válidos
	if (tipo != NULL) 
		*tipo = aux->tipo;
	if (prio != NULL) 
		*prio = aux->prio;

	free(aux);
	return item;		
}

int fprio_tamanho (struct fprio_t *f) {
	if (f == NULL)
		return -1;

	return f->num;
}

void fprio_imprime (struct fprio_t *f) {
	if (f == NULL)
		return;
	
	struct fpnodo_t *aux = f->prim;
	
	if (aux != NULL) {
		printf("(%d %d)", aux->tipo, aux->prio);
		aux = aux->prox;
	}

	while (aux != NULL) {
		printf(" (%d %d)", aux->tipo, aux->prio);
		aux = aux->prox;
	}
}

