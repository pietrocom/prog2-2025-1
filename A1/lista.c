// TAD lista de números inteiros
// Carlos Maziero - DINF/UFPR, Out 2024
//
// Implementação do TAD - a completar
//
// Implementação com lista encadeada dupla não-circular
//
#include <stdio.h>
#include <stdlib.h>
#include "lista.h"

struct lista_t *lista_cria() {
	struct lista_t *lst = malloc(sizeof(struct lista_t));
	if (!lst) 
		return NULL;

	lst->prim = NULL;
	lst->ult = NULL;
	lst->tamanho = 0;
	return lst;
}

struct lista_t *lista_destroi(struct lista_t *lst) {
	while (lst->prim != NULL) {
		struct item_t *aux = lst->prim;
		lst->prim = lst->prim->prox;
		free(aux);
	}
	free(lst);
	return NULL;
}

int lista_insere(struct lista_t *lst, int item, int pos) {
	if (lst == NULL)
		return -1;

	struct item_t *novo = malloc(sizeof(struct item_t));
	if (!novo) 
		return -1;

	novo->valor = item;

	// Caso a lista esteja vazia
	if (lst->prim == NULL) {
		lst->prim = novo;
		lst->ult = novo;
		novo->prox = NULL;
		novo->ant = NULL;
		lst->tamanho++;
		return lst->tamanho;
	}

	// Insercao no final ou caso pos seja invalido
	if (pos >= lst->tamanho || pos == -1) {
		lst->ult->prox = novo;
		novo->ant = lst->ult;
		novo->prox = NULL;
		lst->ult = novo;
		lst->tamanho++;
		return lst->tamanho;
	}

	// Incercao no inicio
	if (pos == 0) {
		novo->prox = lst->prim;
		novo->ant = NULL;
		lst->prim->ant = novo;
		lst->prim = novo;
		lst->tamanho++;
		return lst->tamanho;
	}

	struct item_t *atual = lst->prim;
	for (int i = 0; i < pos; i++) {
		atual = atual->prox;
	}
	novo->prox = atual;
	novo->ant = atual->ant;
	atual->ant->prox = novo;
	atual->ant = novo;
	lst->tamanho++;
	return lst->tamanho;
}

int lista_retira(struct lista_t *lst, int *item, int pos) {
	// Verifica se lista eh nula ou pos invalido
	if (lst == NULL || pos < -1 || pos >= lst->tamanho)
		return -1;

	struct item_t *aux;
	// Retira do final
	if (pos == -1) {
		*item = lst->ult->valor;
		aux = lst->ult;
		lst->ult = aux->ant;
		if (lst->ult) lst->ult->prox = NULL;
		else lst->prim = NULL;
	}
	// Retira do inicio
	else if (pos == 0) {
		aux = lst->prim;
		*item = aux->valor;
		lst->prim = aux->prox;
		if (lst->prim) lst->prim->ant = NULL;
		else lst->ult = NULL;
	}

	else {
		aux = lst->prim;
		for (int i = 0; i < pos; i++) aux = aux->prox;
		*item = aux->valor;
		aux->ant->prox = aux->prox;
		aux->prox->ant = aux->ant;
	}
	free(aux);
	lst->tamanho--;
	return lst->tamanho;
}

int lista_consulta(struct lista_t *lst, int *item, int pos) {
	// Verifica se lista eh nula ou pos invalida
	if (lst == NULL || pos < -1 || pos >= lst->tamanho)
		return -1;

	struct item_t *aux = lst->prim;
	if (pos == -1) aux = lst->ult;
	else for (int i = 0; i < pos; i++) aux = aux->prox;

	*item = aux->valor;
	return lst->tamanho;
}

int lista_procura(struct lista_t *lst, int valor) {
	if (lst == NULL) return -1;

	struct item_t *aux = lst->prim;
	int i = 0;
	while (aux != NULL) {
		if (aux->valor == valor) return i;
		aux = aux->prox;
		i++;
	}
	// Se nao encontrar retorna erro
	return -1;  
}

int lista_tamanho(struct lista_t *lst) {
	if (lst == NULL) return -1;
	return lst->tamanho;
}

void lista_imprime(struct lista_t *lst) {
	struct item_t *aux = lst->prim;

	if (aux != NULL) {
		printf("%d", aux->valor);
		aux = aux->prox;
	}

	while (aux != NULL) {
		printf(" %d", aux->valor);
		aux = aux->prox;
	}
}
