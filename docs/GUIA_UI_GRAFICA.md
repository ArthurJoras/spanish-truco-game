# Guia de Implementação para Interface Gráfica

Este documento descreve como adicionar uma interface gráfica ao jogo de truco mantendo a estrutura existente.

## Arquitetura Atual

O projeto está estruturado em camadas:

```
┌─────────────────────────────────────┐
│     Interface (Terminal/GUI)        │
├─────────────────────────────────────┤
│      Comunicação (cliente.c)        │
├─────────────────────────────────────┤
│      Protocolo (common.h)           │
├─────────────────────────────────────┤
│   Lógica do Jogo (game_logic.c)    │
└─────────────────────────────────────┘
```

## Estratégia de Implementação

### 1. Criar Camada de Abstração de UI

Crie um arquivo `ui_interface.h` com funções abstratas:

```c
typedef struct {
    void (*exibir_menu_principal)(EstadoJogo* estado);
    void (*exibir_carta)(Carta carta);
    void (*exibir_estado_jogo)(EstadoJogo* estado);
    void (*exibir_mensagem)(const char* mensagem);
    int (*obter_escolha_jogador)(int min, int max);
    void (*atualizar_tela)();
} UIInterface;
```

### 2. Implementar UI de Terminal

Mova o código atual de interface para `ui_terminal.c`:

```c
UIInterface criar_ui_terminal() {
    UIInterface ui;
    ui.exibir_menu_principal = terminal_exibir_menu;
    ui.exibir_carta = terminal_exibir_carta;
    // ... outras funções
    return ui;
}
```

### 3. Implementar UI Gráfica

Crie `ui_grafica.c` usando SDL2/GTK/Qt:

```c
UIInterface criar_ui_grafica() {
    UIInterface ui;
    ui.exibir_menu_principal = grafica_exibir_menu;
    ui.exibir_carta = grafica_exibir_carta;
    // ... outras funções
    return ui;
}
```

## Opções de Framework Gráfico

### SDL2 (Recomendado)

**Vantagens:**

- Simples de usar
- Multiplataforma
- Boa para jogos 2D
- Comunidade ativa

**Exemplo básico:**

```c
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
} GUIContext;

void inicializar_gui(GUIContext* ctx) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    ctx->window = SDL_CreateWindow(
        "Truco Espanhol",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_SHOWN
    );

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    ctx->font = TTF_OpenFont("font.ttf", 24);
}

void desenhar_carta(GUIContext* ctx, Carta carta, int x, int y) {
    SDL_Texture* carta_texture = carregar_carta_texture(ctx, carta);
    SDL_Rect dest = {x, y, 100, 150};
    SDL_RenderCopy(ctx->renderer, carta_texture, NULL, &dest);
}
```

**Instalação no Linux:**

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### GTK+ 3

**Vantagens:**

- Interface nativa Linux
- Widgets prontos
- Suporte a CSS para estilização

**Exemplo básico:**

```c
#include <gtk/gtk.h>

typedef struct {
    GtkWidget* window;
    GtkWidget* area_jogo;
    GtkWidget* area_cartas;
} GTKContext;

void inicializar_gtk(GTKContext* ctx, int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    ctx->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(ctx->window), "Truco Espanhol");
    gtk_window_set_default_size(GTK_WINDOW(ctx->window), 1024, 768);

    // ... criar widgets
}
```

**Instalação:**

```bash
sudo apt-get install libgtk-3-dev
```

## Estrutura de Arquivos Sugerida

```
trabalho_truco/
├── common.h              # Estruturas compartilhadas
├── common.c
├── game_logic.h          # Lógica do jogo
├── game_logic.c
├── servidor.c            # Servidor (não muda)
├── cliente_core.h        # Core do cliente (sem UI)
├── cliente_core.c
├── ui_interface.h        # Interface abstrata de UI
├── ui_terminal.h         # Implementação terminal
├── ui_terminal.c
├── ui_grafica.h          # Implementação gráfica
├── ui_grafica.c
├── cliente_terminal.c    # Main do cliente terminal
├── cliente_grafico.c     # Main do cliente gráfico
├── assets/               # Recursos gráficos
│   ├── cartas/          # Imagens das cartas
│   ├── backgrounds/     # Fundos
│   └── fonts/           # Fontes
└── Makefile
```

## Modificações no Makefile

```makefile
# Adicionar targets para versão gráfica
CLIENTE_GUI = cliente_grafico

# Flags para SDL2
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf

# Compilar cliente gráfico
$(CLIENTE_GUI): cliente_grafico.o cliente_core.o ui_grafica.o $(COMMON_OBJ)
	$(CC) $(LDFLAGS) $(SDL_LDFLAGS) -o $@ $^

ui_grafica.o: ui_grafica.c ui_interface.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@
```

## Refatoração do cliente.c

### Separar lógica de apresentação:

**cliente_core.c:**

```c
// Funções de comunicação e lógica
bool conectar_servidor(const char* ip, int porta);
void processar_mensagem(Mensagem* msg);
void enviar_jogada(int carta_index);
// ...
```

**ui_terminal.c:**

```c
// Funções de interface
void exibir_menu();
void exibir_cartas();
// ...
```

## Assets Necessários para UI Gráfica

### Imagens das Cartas

Crie ou baixe imagens PNG para:

- 40 cartas do baralho espanhol
- Verso da carta
- Dimensões recomendadas: 200x300px

### Outros Assets

- **Fundo da mesa**: Verde (estilo mesa de cartas)
- **Botões**: Truco, Envido, Flor, Desistir
- **Indicadores**: Vez do jogador, mão/pé
- **Fonte**: Para pontuação e textos

## Exemplo de Fluxo com GUI

```c
// cliente_grafico.c
int main(int argc, char* argv[]) {
    // Inicializar GUI
    GUIContext gui;
    inicializar_gui(&gui);

    // Inicializar core do cliente
    ClienteCore cliente;
    inicializar_cliente(&cliente, callback_atualizar_gui);

    // Loop principal
    bool rodando = true;
    while(rodando) {
        // Processar eventos da GUI
        SDL_Event evento;
        while(SDL_PollEvent(&evento)) {
            if(evento.type == SDL_QUIT) {
                rodando = false;
            }
            processar_evento_gui(&gui, &cliente, &evento);
        }

        // Processar mensagens do servidor
        processar_mensagens_servidor(&cliente);

        // Renderizar
        renderizar_jogo(&gui, &cliente.estado);

        SDL_Delay(16); // ~60 FPS
    }

    // Limpar
    limpar_gui(&gui);
    desconectar_cliente(&cliente);

    return 0;
}
```

## Sistema de Eventos

Implemente um sistema de callbacks para desacoplar comunicação e UI:

```c
typedef void (*EventoCallback)(TipoEvento tipo, void* dados);

typedef struct {
    EventoCallback on_estado_atualizado;
    EventoCallback on_truco_cantado;
    EventoCallback on_carta_jogada;
    EventoCallback on_fim_partida;
} CallbacksCliente;

void registrar_callbacks(ClienteCore* cliente, CallbacksCliente* callbacks);
```

## Próximos Passos

1. ✅ Testar versão terminal atual
2. ⬜ Refatorar cliente separando lógica de UI
3. ⬜ Criar interface abstrata de UI
4. ⬜ Implementar UI terminal usando interface abstrata
5. ⬜ Criar assets gráficos ou baixar sprites
6. ⬜ Implementar UI gráfica com SDL2
7. ⬜ Adicionar animações e efeitos
8. ⬜ Implementar sistema de sons (opcional)

## Recursos Úteis

- **SDL2 Tutorial**: https://lazyfoo.net/tutorials/SDL/
- **Sprites de cartas**: https://opengameart.org/
- **GTK Tutorial**: https://www.gtk.org/docs/
- **Animações**: Usar SDL2_gfx ou implementar próprio sistema

## Considerações de Performance

- Use texturas em vez de renderizar a cada frame
- Cache imagens das cartas
- Use sprites sheets para animações
- Limite FPS para 60
- Thread separada para comunicação de rede

---

**Nota**: Comece simples! Uma versão gráfica básica funcional é melhor que uma versão complexa incompleta. Adicione features progressivamente.
