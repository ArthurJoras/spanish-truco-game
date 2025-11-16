#ifndef UI_GRAFICA_H
#define UI_GRAFICA_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include "common.h"

// Cores
#define COR_FUNDO_R 34
#define COR_FUNDO_G 139
#define COR_FUNDO_B 34
#define COR_TEXTO_R 255
#define COR_TEXTO_G 255
#define COR_TEXTO_B 255
#define COR_BOTAO_R 139
#define COR_BOTAO_G 69
#define COR_BOTAO_B 19
#define COR_BOTAO_HOVER_R 160
#define COR_BOTAO_HOVER_G 82
#define COR_BOTAO_HOVER_B 45

// Dimensões
#define LARGURA_JANELA 1280
#define ALTURA_JANELA 720
#define LARGURA_CARTA 100
#define ALTURA_CARTA 150
#define ESPACAMENTO_CARTA 20

// Estrutura de um botão
typedef struct {
	SDL_Rect rect;
	char texto[64];
	bool visivel;
	bool hover;
	void (*callback)(void*);
	void* callback_data;
} Botao;

// Estrutura de contexto gráfico
typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	TTF_Font* font_titulo;
	TTF_Font* font_normal;
	TTF_Font* font_pequena;

	// Texturas das cartas
	SDL_Texture* cartas[4][13];  // [naipe][numero]
	SDL_Texture* carta_verso;

	// Texturas de fundo
	SDL_Texture* fundo_mesa;

	// Botões
	Botao* botoes;
	int num_botoes;
	int max_botoes;

	// Estado da UI
	int carta_selecionada;
	bool mouse_sobre_carta[3];

	// Posição do mouse
	int mouse_x;
	int mouse_y;

} UIGrafica;

// Enums para telas
typedef enum {
	TELA_MENU_PRINCIPAL,
	TELA_CRIAR_SALA,
	TELA_LISTAR_SALAS,
	TELA_LOBBY,
	TELA_JOGO,
	TELA_FIM_PARTIDA
} TipoTela;

// Estrutura de estado da UI
typedef struct {
	TipoTela tela_atual;
	uint32_t sala_id;
	bool em_partida;
	EstadoJogo estado_jogo;
	InfoSala salas[MAX_SALAS];
	int num_salas;
	char input_texto[256];
	bool aguardando_resposta_canto;
	TipoMensagem tipo_canto_aguardando;
	char mensagem_temporaria[256];
	float tempo_mensagem;
	int vencedor_partida;
	bool precisa_reconfigurar_botoes;
} UIEstado;

// Funções principais
bool ui_inicializar(UIGrafica* ui);
void ui_finalizar(UIGrafica* ui);
void ui_renderizar(UIGrafica* ui, UIEstado* estado);
void ui_processar_evento(UIGrafica* ui, UIEstado* estado, SDL_Event* evento);

// Funções de renderização
void ui_renderizar_menu_principal(UIGrafica* ui, UIEstado* estado);
void ui_renderizar_criar_sala(UIGrafica* ui, UIEstado* estado);
void ui_renderizar_listar_salas(UIGrafica* ui, UIEstado* estado);
void ui_renderizar_lobby(UIGrafica* ui, UIEstado* estado);
void ui_renderizar_jogo(UIGrafica* ui, UIEstado* estado);
void ui_renderizar_fim_partida(UIGrafica* ui, UIEstado* estado);

// Funções auxiliares
void ui_carregar_cartas(UIGrafica* ui);
void ui_desenhar_carta(UIGrafica* ui, Carta carta, int x, int y, bool selecionada, bool hover);
void ui_desenhar_carta_verso(UIGrafica* ui, int x, int y);
void ui_desenhar_texto(UIGrafica* ui, const char* texto, int x, int y, TTF_Font* font, SDL_Color cor);
void ui_desenhar_botao(UIGrafica* ui, Botao* botao);
void ui_desenhar_retangulo(UIGrafica* ui, SDL_Rect rect, SDL_Color cor, bool preenchido);
Botao* ui_adicionar_botao(UIGrafica* ui, int x, int y, int w, int h, const char* texto, void (*callback)(void*), void* data);
void ui_limpar_botoes(UIGrafica* ui);
void ui_atualizar_botoes(UIGrafica* ui, int mouse_x, int mouse_y);
Botao* ui_obter_botao_clicado(UIGrafica* ui, int mouse_x, int mouse_y);

// Funções de mapeamento
const char* obter_nome_arquivo_carta(Naipe naipe, NumeroCarta numero);
int obter_indice_numero_carta(NumeroCarta numero);

#endif  // UI_GRAFICA_H
