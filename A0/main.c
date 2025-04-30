#include "main.h"

// Funcoes

void requisita(char *titulo, char *texto){
	getchar();
	printf("\nDigite o título: ");
	fgets(titulo, 33, stdin);
	printf("Digite o texto: ");
	fgets(texto, 513, stdin);
}

struct filajornal *filacria () {
	struct filajornal *fila = malloc(sizeof(struct filajornal));
	if (!fila)
		return NULL;

	fila->prim = NULL;
	fila->ult = NULL;
	fila->tam = 0;

	return fila;
}

void filainsere (struct filajornal *fila, struct noticia *noticia) {
	if (!fila || !noticia)
		return;

	if (fila->prim == NULL) {
		fila->prim = noticia;
		fila->ult = noticia;
		return;
	}

	fila->ult->prox = noticia;
	noticia->prox = NULL;
	return;
}

struct noticia *filaremove (struct filajornal *fila) {
	if (!fila)
		return NULL;

	struct noticia *aux = fila->prim;

	fila->prim = aux->prox;

	return aux;
}

void filadestroi (struct filajornal *fila) {
	if (!fila)
		return;
	
	struct noticia *aux = fila->prim;

	// Desaloca as noticias da fila
	while (aux != NULL) {
		aux = aux->prox;
		free(fila->prim);
		fila->prim = aux;
	}

	free(fila);
}

struct noticia *crianoticia () {
	struct noticia *nova = malloc(sizeof(struct noticia));
	if (!nova)
		return NULL;

	requisita(nova->titulo, nova->texto);
	nova->idade = 0;
	nova->prox = NULL;
	char prio;
	printf("Digite a prioridade (0 para breaking news ou 1 para noticia normal): ");
	getchar();
	scanf("%c", &prio);
	nova->prio = prio;

	return nova;
}

void filaatualiza (struct filajornal *fila) {
	struct noticia *aux = fila->prim;

	while (aux != NULL) {
		aux->idade++;
	
		// Caso de remocao da fila
		if (aux->idade > 3) {
			struct noticia *aux2 = aux;
			aux = aux->prox;
			free(aux2);
		}
		else
			aux = aux->prox;
	}
}

void cadastranoticia (struct filajornal *fila0, struct filajornal *fila1) {
	if (!fila0 || !fila1)
		return;

	struct noticia *noticia = crianoticia();

	if (noticia->prio == 0)
		filainsere(fila0, noticia);
	else
		filainsere(fila1, noticia);
}

struct noticia *selecionanoticia (struct filajornal *fila0, struct filajornal *fila1) {
	if (fila0->prim != NULL)
		return filaremove(fila0);

	if (fila1->prim != NULL)
		return filaremove(fila1);

	// Retorna NULL caso nao haja noticia
	return NULL;
}

void imprimenoticia (struct noticia *noticia) {
	if (noticia == NULL)
		return;

	printf("=======================================================\n");
	printf("%s\n", noticia->titulo);
	printf("\n");
	printf("%s\n");
	printf("=======================================================\n");
}

void imprimeedicao (struct filajornal *fila0, struct filajornal *fila1) {
	if (!fila0 || !fila1)
		return;

	int cont = 0;
	struct noticia *noticia1 = selecionanoticia(fila0, fila1);
	struct noticia *noticia2 = selecionanoticia(fila0, fila1);
	
	if ( (noticia1 == NULL) && (noticia2 == NULL) ) {
		printf("Esta edicao foi pulada por falta de noticias!");
		return;
	}

	imprimenoticia(noticia1);
	imprimenoticia(noticia2);

	if (noticia1)
		free(noticia1);
	if (noticia2)
		free(noticia2);
}

void imprimeopcoes () {
	printf("- (1)Cadastrar noticia\n");
	printf("- (2)Fechar edicao\n");
	printf("- (3)Sair\n");
}

void encerrasimulacao (struct filajornal *fila0, struct filajornal *fila1) {
	filadestroi(fila0);
	filadestroi(fila1);
}


// Main

int main () {
	
	struct filajornal *fila0 = filacria();
	struct filajornal *fila1 = filacria();

	int status = 1;

	while (status == 1) {
		
		imprimeopcoes();

		int opcao;
		scanf("%d", &opcao);

		switch (opcao) {
			case 1:				
				cadastranoticia(fila0, fila1);
				break;

			case 2:
				imprimeedicao(fila0, fila1);
				filaatualiza(fila0);
				filaatualiza(fila1);
				break;

			case 3:
				encerrasimulacao(fila0, fila1);
				status = 0;
				break;

			break;
		}
	}

	return 0;	
}
