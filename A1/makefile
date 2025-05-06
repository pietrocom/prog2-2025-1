# Compilador
CC := gcc
# Flags
CFLAGS := -Wall -Wextra -g -I./include
# Diretorios
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/objs
LOGIN_DIR := login
INCLUDE_DIR := include

# Arquivos fonte
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/lz/*.c)
# Arquivos objeto
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
# Executavel final
TARGET := $(LOGIN_DIR)/vina

# Regra padrao
all: $(TARGET)

# Regra para o executavel (em login/)
$(TARGET): $(OBJS) | $(LOGIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Regra generica para .c -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)  # Cria subdiretórios (ex: objs/lz/)
	$(CC) $(CFLAGS) -c $< -o $@

# Cria diretorios necessarios
$(LOGIN_DIR):
	mkdir -p $@
$(OBJ_DIR):
	mkdir -p $@

# Dependencias (ajuste conforme necessario)
$(OBJ_DIR)/main.o: $(INCLUDE_DIR)/vina.h $(INCLUDE_DIR)/types.h
$(OBJ_DIR)/lz/lz.o: $(INCLUDE_DIR)/lz/lz.h $(INCLUDE_DIR)/types.h
# (Adicione outras dependencias aqui)

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(LOGIN_DIR)

.PHONY: all clean