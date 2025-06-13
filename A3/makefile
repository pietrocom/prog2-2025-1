# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -Wextra -Iinclude -g $(shell pkg-config --cflags allegro-5 allegro_image-5 allegro_font-5 allegro_ttf-5)

# Flags de linkedição
LDFLAGS = $(shell pkg-config --libs allegro-5 allegro_image-5 allegro_font-5 allegro_ttf-5)

# Nome do executável
TARGET = run_and_gun

# Diretórios
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Arquivos fonte
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Regra principal
all: create_dirs $(BINDIR)/$(TARGET)

# Cria os diretórios necessários
create_dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# Compila o executável
$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compila cada arquivo objeto
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Instala as dependências (Linux/Ubuntu)
install_deps:
	sudo apt-get install -y liballegro5-dev liballegro-image5-dev liballegro-font5-dev liballegro-ttf5-dev

# Executa o jogo
run: all
	./$(BINDIR)/$(TARGET)

.PHONY: all clean create_dirs install_deps run