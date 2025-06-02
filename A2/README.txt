Pietro Comin - GRR20241955


A função que define o -z está no arquivo src/options.c, enquanto a main esta em src/main.c.


1. Validação Inicial (main.c -> a partir da linha 107)

    Verifica se o arquivo .vc existe (abre em modo "rb")

    Cria um novo nome para o arquivo derivado adicionando _z antes da extensão

    Cria e abre o novo arquivo em modo "w+b" (escrita binária + leitura)


2. Preparação das Estruturas

    Cria um novo diretório (estrutura que armazena metadados) para o arquivo derivado

    Inicializa o diretório com os metadados básicos


3. Processamento dos Membros (função z() options.c -> 411)

Para cada membro especificado na linha de comando:

    Verifica existência: Confere se o membro existe no arquivo original

    Prepara buffer: Aloca memória para armazenar o conteúdo do membro

    Lê conteúdo: Extrai o conteúdo do membro do arquivo original

    Atualiza metadados: Insere a struct nova no novo diretorio

    Ajusta offsets: Move os dados existentes para acomodar os novos metadados e ajusta offsets

    Escreve conteúdo: Grava o conteúdo do membro no novo arquivo


4. Fluxo de Dados

    Arquivo original → Buffer → Novo arquivo

    Metadados originais → Novo diretório → Novo arquivo

