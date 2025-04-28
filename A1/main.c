#include "vina.h"
#include "utils.h"
#include "options.h"
#include "aux.h"

// Função auxiliar para imprimir informações de um arquivo
void print_arquivo_info(struct arquivo *arq) {
    if (!arq) {
        printf("Arquivo NULL\n");
        return;
    }
    
    printf("Nome: %s\n", arq->nome);
    printf("UID: %d\n", arq->uid);
    printf("Tamanho original: %lu\n", arq->tam_or);
    printf("Tamanho comprimido: %lu\n", arq->tam_comp);
    printf("Ordem: %d\n", arq->ordem);
    printf("Offset: %lu\n", arq->offset);
    printf("Mod time: %ld\n", arq->mod_time);
    printf("-----------------\n");
}

// Função auxiliar para imprimir informações do diretório
void print_diretorio_info(struct diretorio *dir) {
    if (!dir) {
        printf("Diretorio NULL\n");
        return;
    }
    
    printf("Quantidade de membros: %d\n", dir->qtd_membros);
    for (int i = 0; i < dir->qtd_membros; i++) {
        printf("Membro %d:\n", i);
        print_arquivo_info(dir->membros[i]);
    }
    printf("=================\n");
}

// Teste básico para criação e destruição de estruturas
void test_criacao_destruicao() {
    printf("=== TESTE CRIAÇÃO/DESTRUIÇÃO ===\n");
    
    // Teste struct arquivo
    struct arquivo *arq = cria_s_arquivo();
    if (!arq) {
        printf("Falha ao criar struct arquivo\n");
        return;
    }
    
    printf("Struct arquivo criada com sucesso\n");
    print_arquivo_info(arq);
    destroi_s_arquivo(arq);
    printf("Struct arquivo destruída com sucesso\n");
    
    // Teste struct diretorio
    struct diretorio *dir = cria_diretorio();
    if (!dir) {
        printf("Falha ao criar struct diretorio\n");
        return;
    }
    
    printf("Struct diretorio criada com sucesso\n");
    print_diretorio_info(dir);
    destroi_diretorio(dir);
    printf("Struct diretorio destruída com sucesso\n");
}

// Teste para manipulação de arquivos
void test_manipulacao_arquivos() {
    printf("\n=== TESTE MANIPULAÇÃO DE ARQUIVOS ===\n");
    
    // Criar um arquivo de teste
    const char *test_filename = "test_file.txt";
    FILE *test_file = fopen(test_filename, "w");
    if (!test_file) {
        printf("Falha ao criar arquivo de teste\n");
        return;
    }
    fprintf(test_file, "Conteudo de teste");
    fclose(test_file);
    
    // Testar criação e inicialização de struct arquivo
    struct arquivo *arq = cria_s_arquivo();
    if (!arq) {
        printf("Falha ao criar struct arquivo\n");
        return;
    }
    
    arq = inicia_s_arquivo(arq, (char *)test_filename);
    if (!arq) {
        printf("Falha ao inicializar struct arquivo\n");
        return;
    }
    
    printf("Arquivo inicializado com sucesso:\n");
    print_arquivo_info(arq);
    
    // Testar diretório
    struct diretorio *dir = cria_diretorio();
    if (!dir) {
        printf("Falha ao criar diretorio\n");
        return;
    }
    
    // Inserir arquivo no diretório
    if (insere_s_arquivo(dir, arq, -1) != 0) {
        printf("Falha ao inserir arquivo no diretorio\n");
        return;
    }
    
    printf("Arquivo inserido no diretorio com sucesso:\n");
    print_diretorio_info(dir);
    
    // Atualizar metadados
    atualiza_metadados(dir);
    printf("Metadados atualizados:\n");
    print_diretorio_info(dir);
    
    // Limpeza
    destroi_diretorio(dir); // Isso também libera o arquivo
    remove(test_filename);
}

// Teste para arquivo archiver
void test_archiver() {
    printf("\n=== TESTE ARCHIVER ===\n");
    
    const char *archive_name = "test_archive.vina";
    const char *test_file1 = "test1.txt";
    const char *test_file2 = "test2.txt";
    
    // Criar arquivos de teste
    FILE *f1 = fopen(test_file1, "w");
    FILE *f2 = fopen(test_file2, "w");
    if (!f1 || !f2) {
        printf("Falha ao criar arquivos de teste\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }
    fprintf(f1, "Conteudo do arquivo 1");
    fprintf(f2, "Conteudo do arquivo 2");
    fclose(f1);
    fclose(f2);
    
    // Criar archiver
    FILE *archive = cria_arquivo((char *)archive_name);
    if (!archive) {
        printf("Falha ao criar archiver\n");
        return;
    }
    fclose(archive);
    
    // Criar e inicializar diretorio
    struct diretorio *dir = cria_diretorio();
    if (!dir) {
        printf("Falha ao criar diretorio\n");
        return;
    }
    
    if (inicia_diretorio(dir, (char *)archive_name)) {
        printf("Falha ao inicializar diretorio\n");
        return;
    }
    
    printf("Diretorio inicializado (deve estar vazio):\n");
    print_diretorio_info(dir);
    
    // Testar operação IP (inserir)
    printf("Testando operacao IP...\n");
    if (ip(dir, (char *)test_file1, (char *)archive_name) != 0) {
        printf("Falha na operacao IP para %s\n", test_file1);
        return;
    }
    
    printf("Arquivo %s inserido com sucesso:\n", test_file1);
    print_diretorio_info(dir);
    
    // Tentar inserir novamente (deve atualizar)
    printf("Tentando inserir novamente (deve atualizar)...\n");
    if (ip(dir, (char *)test_file1, (char *)archive_name) != 0) {
        printf("Falha na operacao IP para %s (segunda vez)\n", test_file1);
        return;
    }
    
    // Inserir segundo arquivo
    if (ip(dir, (char *)test_file2, (char *)archive_name) != 0) {
        printf("Falha na operacao IP para %s\n", test_file2);
        return;
    }
    
    printf("Arquivo %s inserido com sucesso:\n", test_file2);
    print_diretorio_info(dir);
    
    // Limpeza
    destroi_diretorio(dir);
    remove(test_file1);
    remove(test_file2);
    remove(archive_name);
}

/*
int main() {
    printf("=== INICIANDO TESTES ===\n");
    
    // Teste 1: Criação e destruição de estruturas
    test_criacao_destruicao();
    
    // Teste 2: Manipulação de arquivos
    test_manipulacao_arquivos();
    
    // Teste 3: Operações com archiver
    test_archiver();
    
    printf("=== TESTES CONCLUÍDOS ===\n");
    return 0;
}
*/

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

