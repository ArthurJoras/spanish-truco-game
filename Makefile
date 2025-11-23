# Makefile - Truco Espanhol Multijogador
# Estrutura organizada com diretórios separados

CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LDFLAGS = -pthread

# Diretórios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
ASSETS_DIR = assets

# Flags SDL2
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf

# Adiciona include path
CFLAGS += -I$(INC_DIR)

# Arquivos fonte
COMMON_SRC = $(SRC_DIR)/common.c
GAME_SRC = $(SRC_DIR)/game_logic.c
SERVER_SRC = $(SRC_DIR)/servidor.c
CLIENT_GRAFICO_SRC = $(SRC_DIR)/cliente_grafico.c
UI_GRAFICA_SRC = $(SRC_DIR)/ui_grafica.c

# Arquivos objeto (no build/)
COMMON_OBJ = $(BUILD_DIR)/common.o
GAME_OBJ = $(BUILD_DIR)/game_logic.o
SERVER_OBJ = $(BUILD_DIR)/servidor.o
CLIENT_GRAFICO_OBJ = $(BUILD_DIR)/cliente_grafico.o
UI_GRAFICA_OBJ = $(BUILD_DIR)/ui_grafica.o

# Executáveis (no build/)
SERVER = $(BUILD_DIR)/servidor
CLIENT_GRAFICO = $(BUILD_DIR)/cliente_grafico

# Target padrão
all: $(SERVER) $(CLIENT_GRAFICO)

# Criar diretório build se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Executáveis
$(SERVER): $(SERVER_OBJ) $(GAME_OBJ) $(COMMON_OBJ) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(CLIENT_GRAFICO): $(CLIENT_GRAFICO_OBJ) $(UI_GRAFICA_OBJ) $(COMMON_OBJ) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(SDL_LDFLAGS)

# Compilação dos objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilação de objetos com SDL2
$(UI_GRAFICA_OBJ): $(UI_GRAFICA_SRC) $(INC_DIR)/ui_grafica.h $(INC_DIR)/common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(CLIENT_GRAFICO_OBJ): $(CLIENT_GRAFICO_SRC) $(INC_DIR)/ui_grafica.h $(INC_DIR)/common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Dependências
$(SERVER_OBJ): $(SERVER_SRC) $(INC_DIR)/common.h $(INC_DIR)/game_logic.h
$(GAME_OBJ): $(GAME_SRC) $(INC_DIR)/game_logic.h $(INC_DIR)/common.h
$(COMMON_OBJ): $(COMMON_SRC) $(INC_DIR)/common.h

# Limpeza
clean:
	rm -rf $(BUILD_DIR)/*

# Executar servidor
run-server: $(SERVER)
	./$(SERVER)

# Executar cliente gráfico
run-client: $(CLIENT_GRAFICO)
	./$(CLIENT_GRAFICO)

# Executar servidor em background
demo: all
	@echo "Iniciando servidor..."
	./$(SERVER) & echo $$! > $(BUILD_DIR)/server.pid
	@sleep 1
	@echo "Servidor iniciado! PID salvo em build/server.pid"
	@echo ""
	@echo "Para conectar clientes, execute:"
	@echo "  make run-client"

# Parar servidor
stop-server:
	@if [ -f $(BUILD_DIR)/server.pid ]; then \
		kill `cat $(BUILD_DIR)/server.pid` 2>/dev/null || true; \
		rm -f $(BUILD_DIR)/server.pid; \
		echo "Servidor parado."; \
	else \
		echo "Nenhum servidor rodando."; \
	fi

# Instalar dependências (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev gcc make

# Help
help:
	@echo "==================================================="
	@echo "  Makefile - Truco Espanhol Multijogador"
	@echo "==================================================="
	@echo ""
	@echo "Estrutura do Projeto:"
	@echo "  src/        - Código fonte (.c)"
	@echo "  include/    - Headers (.h)"
	@echo "  build/      - Executáveis e objetos compilados"
	@echo "  assets/     - Imagens das cartas (PNG)"
	@echo "  docs/       - Documentação"
	@echo "  scripts/    - Scripts auxiliares"
	@echo ""
	@echo "Targets Disponíveis:"
	@echo "  all              - Compila tudo (padrão)"
	@echo "  servidor         - Compila apenas o servidor"
	@echo "  cliente_grafico  - Compila apenas o cliente gráfico"
	@echo "  clean            - Remove arquivos compilados"
	@echo "  run-server       - Compila e executa o servidor"
	@echo "  run-client       - Compila e executa o cliente gráfico"
	@echo "  demo             - Inicia servidor em background"
	@echo "  stop-server      - Para o servidor em background"
	@echo "  install-deps     - Instala dependências no Ubuntu/Debian"
	@echo "  help             - Mostra esta ajuda"
	@echo ""
	@echo "Exemplos de Uso:"
	@echo "  make                       # Compila tudo"
	@echo "  make run-server            # Inicia servidor"
	@echo "  make run-client            # Inicia cliente gráfico"
	@echo "  make clean && make         # Recompila do zero"
	@echo ""
	@echo "Executáveis compilados ficam em: $(BUILD_DIR)/"
	@echo "  ./$(SERVER) [porta]"
	@echo "  ./$(CLIENT_GRAFICO) [ip] [porta]"
	@echo ""
	@echo "==================================================="

.PHONY: all clean run-server run-client demo stop-server install-deps help
