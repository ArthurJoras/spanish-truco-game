#include "ui_grafica.h"

#include <stdio.h>
#include <string.h>

// Mapeamento de naipes para nomes de arquivo
const char* obter_nome_naipe_arquivo(Naipe naipe) {
	switch (naipe) {
		case NAIPE_ESPADAS:
			return "espadas";
		case NAIPE_PAUS:
			return "paus";
		case NAIPE_COPAS:
			return "copas";
		case NAIPE_OUROS:
			return "ouros";
		default:
			return "espadas";
	}
}

// Obter índice do número da carta no array
int obter_indice_numero_carta(NumeroCarta numero) {
	switch (numero) {
		case NUMERO_AS:
			return 0;
		case NUMERO_2:
			return 1;
		case NUMERO_3:
			return 2;
		case NUMERO_4:
			return 3;
		case NUMERO_5:
			return 4;
		case NUMERO_6:
			return 5;
		case NUMERO_7:
			return 6;
		case NUMERO_10:
			return 7;
		case NUMERO_11:
			return 8;
		case NUMERO_12:
			return 9;
		default:
			return 0;
	}
}

// Obter número da carta para nome de arquivo
int obter_numero_arquivo(NumeroCarta numero) {
	return (int)numero;
}

const char* obter_nome_arquivo_carta(Naipe naipe, NumeroCarta numero) {
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "assets/img/%02d-%s.png",
	         obter_numero_arquivo(numero), obter_nome_naipe_arquivo(naipe));
	return buffer;
}

bool ui_inicializar(UIGrafica* ui) {
	memset(ui, 0, sizeof(UIGrafica));

	// Inicializar SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Erro ao inicializar SDL: %s\n", SDL_GetError());
		return false;
	}

	// Inicializar SDL_image
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		fprintf(stderr, "Erro ao inicializar SDL_image: %s\n", IMG_GetError());
		SDL_Quit();
		return false;
	}

	// Inicializar SDL_ttf
	if (TTF_Init() < 0) {
		fprintf(stderr, "Erro ao inicializar SDL_ttf: %s\n", TTF_GetError());
		IMG_Quit();
		SDL_Quit();
		return false;
	}

	// Criar janela
	ui->window = SDL_CreateWindow(
	    "Truco Espanhol",
	    SDL_WINDOWPOS_CENTERED,
	    SDL_WINDOWPOS_CENTERED,
	    LARGURA_JANELA,
	    ALTURA_JANELA,
	    SDL_WINDOW_SHOWN);

	if (!ui->window) {
		fprintf(stderr, "Erro ao criar janela: %s\n", SDL_GetError());
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return false;
	}

	// Criar renderer
	ui->renderer = SDL_CreateRenderer(ui->window, -1,
	                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!ui->renderer) {
		fprintf(stderr, "Erro ao criar renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(ui->window);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return false;
	}

	// Carregar fontes (usando fonte padrão do sistema)
	// Tenta carregar uma fonte comum do Linux
	const char* fontes[] = {
	    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
	    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
	    "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
	    NULL};

	ui->font_titulo = NULL;
	for (int i = 0; fontes[i] != NULL; i++) {
		ui->font_titulo = TTF_OpenFont(fontes[i], 48);
		if (ui->font_titulo) break;
	}

	ui->font_normal = NULL;
	for (int i = 0; fontes[i] != NULL; i++) {
		ui->font_normal = TTF_OpenFont(fontes[i], 24);
		if (ui->font_normal) break;
	}

	ui->font_pequena = NULL;
	for (int i = 0; fontes[i] != NULL; i++) {
		ui->font_pequena = TTF_OpenFont(fontes[i], 18);
		if (ui->font_pequena) break;
	}

	if (!ui->font_titulo || !ui->font_normal || !ui->font_pequena) {
		fprintf(stderr, "Aviso: Não foi possível carregar fontes do sistema\n");
	}

	// Inicializar array de botões
	ui->max_botoes = 20;
	ui->botoes = malloc(sizeof(Botao) * ui->max_botoes);
	ui->num_botoes = 0;

	// Carregar cartas
	ui_carregar_cartas(ui);

	ui->carta_selecionada = -1;

	printf("UI gráfica inicializada com sucesso!\n");
	return true;
}

void ui_carregar_cartas(UIGrafica* ui) {
	// Carregar todas as cartas
	Naipe naipes[] = {NAIPE_ESPADAS, NAIPE_PAUS, NAIPE_COPAS, NAIPE_OUROS};
	NumeroCarta numeros[] = {NUMERO_AS, NUMERO_2, NUMERO_3, NUMERO_4, NUMERO_5,
	                         NUMERO_6, NUMERO_7, NUMERO_10, NUMERO_11, NUMERO_12};

	for (int n = 0; n < 4; n++) {
		for (int c = 0; c < 10; c++) {
			const char* arquivo = obter_nome_arquivo_carta(naipes[n], numeros[c]);
			SDL_Surface* surface = IMG_Load(arquivo);

			if (surface) {
				ui->cartas[n][obter_indice_numero_carta(numeros[c])] =
				    SDL_CreateTextureFromSurface(ui->renderer, surface);
				SDL_FreeSurface(surface);
			} else {
				fprintf(stderr, "Aviso: Não foi possível carregar %s\n", arquivo);
				ui->cartas[n][obter_indice_numero_carta(numeros[c])] = NULL;
			}
		}
	}

	// Carregar verso da carta
	SDL_Surface* verso_surface = IMG_Load("assets/img/reverso.png");
	if (verso_surface) {
		ui->carta_verso = SDL_CreateTextureFromSurface(ui->renderer, verso_surface);
		SDL_FreeSurface(verso_surface);
	} else {
		fprintf(stderr, "Aviso: Não foi possível carregar verso da carta\n");
		ui->carta_verso = NULL;
	}

	printf("Cartas carregadas!\n");
}

void ui_finalizar(UIGrafica* ui) {
	// Liberar texturas das cartas
	for (int n = 0; n < 4; n++) {
		for (int c = 0; c < 13; c++) {
			if (ui->cartas[n][c]) {
				SDL_DestroyTexture(ui->cartas[n][c]);
			}
		}
	}

	if (ui->carta_verso) {
		SDL_DestroyTexture(ui->carta_verso);
	}

	// Liberar fontes
	if (ui->font_titulo) TTF_CloseFont(ui->font_titulo);
	if (ui->font_normal) TTF_CloseFont(ui->font_normal);
	if (ui->font_pequena) TTF_CloseFont(ui->font_pequena);

	// Liberar botões
	if (ui->botoes) free(ui->botoes);

	// Liberar SDL
	if (ui->renderer) SDL_DestroyRenderer(ui->renderer);
	if (ui->window) SDL_DestroyWindow(ui->window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void ui_desenhar_texto(UIGrafica* ui, const char* texto, int x, int y, TTF_Font* font, SDL_Color cor) {
	if (!font) return;

	SDL_Surface* surface = TTF_RenderText_Blended(font, texto, cor);
	if (!surface) return;

	SDL_Texture* texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
	if (!texture) {
		SDL_FreeSurface(surface);
		return;
	}

	SDL_Rect dest = {x, y, surface->w, surface->h};
	SDL_RenderCopy(ui->renderer, texture, NULL, &dest);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void ui_desenhar_retangulo(UIGrafica* ui, SDL_Rect rect, SDL_Color cor, bool preenchido) {
	SDL_SetRenderDrawColor(ui->renderer, cor.r, cor.g, cor.b, cor.a);

	if (preenchido) {
		SDL_RenderFillRect(ui->renderer, &rect);
	} else {
		SDL_RenderDrawRect(ui->renderer, &rect);
	}
}

void ui_desenhar_botao(UIGrafica* ui, Botao* botao) {
	if (!botao->visivel) return;

	SDL_Color cor_fundo = botao->hover ? (SDL_Color){COR_BOTAO_HOVER_R, COR_BOTAO_HOVER_G, COR_BOTAO_HOVER_B, 255} : (SDL_Color){COR_BOTAO_R, COR_BOTAO_G, COR_BOTAO_B, 255};

	ui_desenhar_retangulo(ui, botao->rect, cor_fundo, true);

	SDL_Color cor_borda = {0, 0, 0, 255};
	ui_desenhar_retangulo(ui, botao->rect, cor_borda, false);

	SDL_Color cor_texto = {COR_TEXTO_R, COR_TEXTO_G, COR_TEXTO_B, 255};

	// Calcula tamanho real do texto para centralizar corretamente
	int texto_w = 0, texto_h = 0;
	if (ui->font_normal) {
		TTF_SizeText(ui->font_normal, botao->texto, &texto_w, &texto_h);
	}
	int texto_x = botao->rect.x + (botao->rect.w - texto_w) / 2;
	int texto_y = botao->rect.y + (botao->rect.h - texto_h) / 2;

	ui_desenhar_texto(ui, botao->texto, texto_x, texto_y, ui->font_normal, cor_texto);
}

Botao* ui_adicionar_botao(UIGrafica* ui, int x, int y, int w, int h,
                          const char* texto, void (*callback)(void*), void* data) {
	if (ui->num_botoes >= ui->max_botoes) {
		printf("ERRO: Máximo de botões atingido!\n");
		return NULL;
	}

	Botao* botao = &ui->botoes[ui->num_botoes++];
	botao->rect = (SDL_Rect){x, y, w, h};
	strncpy(botao->texto, texto, sizeof(botao->texto) - 1);
	botao->visivel = true;
	botao->hover = false;
	botao->callback = callback;
	botao->callback_data = data;

	return botao;
}

void ui_limpar_botoes(UIGrafica* ui) {
	ui->num_botoes = 0;
}

void ui_atualizar_botoes(UIGrafica* ui, int mouse_x, int mouse_y) {
	ui->mouse_x = mouse_x;
	ui->mouse_y = mouse_y;

	for (int i = 0; i < ui->num_botoes; i++) {
		Botao* botao = &ui->botoes[i];
		if (!botao->visivel) continue;

		botao->hover = (mouse_x >= botao->rect.x &&
		                mouse_x <= botao->rect.x + botao->rect.w &&
		                mouse_y >= botao->rect.y &&
		                mouse_y <= botao->rect.y + botao->rect.h);
	}
}

Botao* ui_obter_botao_clicado(UIGrafica* ui, int mouse_x, int mouse_y) {
	for (int i = 0; i < ui->num_botoes; i++) {
		Botao* botao = &ui->botoes[i];
		if (!botao->visivel) continue;

		if (mouse_x >= botao->rect.x &&
		    mouse_x <= botao->rect.x + botao->rect.w &&
		    mouse_y >= botao->rect.y &&
		    mouse_y <= botao->rect.y + botao->rect.h) {
			return botao;
		}
	}
	return NULL;
}

void ui_desenhar_carta(UIGrafica* ui, Carta carta, int x, int y, bool selecionada, bool hover) {
	SDL_Texture* texture = ui->cartas[carta.naipe][obter_indice_numero_carta(carta.numero)];

	if (!texture) {
		// Desenha retângulo se não tem textura
		SDL_Rect rect = {x, y, LARGURA_CARTA, ALTURA_CARTA};
		SDL_Color cor = {200, 200, 200, 255};
		ui_desenhar_retangulo(ui, rect, cor, true);
		return;
	}

	int offset_y = selecionada ? -20 : (hover ? -10 : 0);
	SDL_Rect dest = {x, y + offset_y, LARGURA_CARTA, ALTURA_CARTA};

	SDL_RenderCopy(ui->renderer, texture, NULL, &dest);

	if (selecionada || hover) {
		SDL_SetRenderDrawColor(ui->renderer, 255, 255, 0, 255);
		SDL_RenderDrawRect(ui->renderer, &dest);
		SDL_RenderDrawRect(ui->renderer, &(SDL_Rect){dest.x + 1, dest.y + 1, dest.w - 2, dest.h - 2});
	}
}

void ui_desenhar_carta_verso(UIGrafica* ui, int x, int y) {
	if (!ui->carta_verso) {
		SDL_Rect rect = {x, y, LARGURA_CARTA, ALTURA_CARTA};
		SDL_Color cor = {100, 100, 150, 255};
		ui_desenhar_retangulo(ui, rect, cor, true);
		return;
	}

	SDL_Rect dest = {x, y, LARGURA_CARTA, ALTURA_CARTA};
	SDL_RenderCopy(ui->renderer, ui->carta_verso, NULL, &dest);
}

void ui_renderizar(UIGrafica* ui, UIEstado* estado) {
	// Limpar tela com cor de fundo (verde mesa)
	SDL_SetRenderDrawColor(ui->renderer, COR_FUNDO_R, COR_FUNDO_G, COR_FUNDO_B, 255);
	SDL_RenderClear(ui->renderer);

	// Renderizar tela apropriada
	switch (estado->tela_atual) {
		case TELA_MENU_PRINCIPAL:
			ui_renderizar_menu_principal(ui, estado);
			break;
		case TELA_CRIAR_SALA:
			ui_renderizar_criar_sala(ui, estado);
			break;
		case TELA_LISTAR_SALAS:
			ui_renderizar_listar_salas(ui, estado);
			break;
		case TELA_LOBBY:
			ui_renderizar_lobby(ui, estado);
			break;
		case TELA_JOGO:
			ui_renderizar_jogo(ui, estado);
			break;
		case TELA_FIM_PARTIDA:
			ui_renderizar_fim_partida(ui, estado);
			break;
	}

	// Renderizar botões
	for (int i = 0; i < ui->num_botoes; i++) {
		ui_desenhar_botao(ui, &ui->botoes[i]);
	}

	// Renderizar mensagem temporária (se houver)
	if (estado->tempo_mensagem > 0) {
		SDL_Color cor_msg = {255, 255, 0, 255};
		ui_desenhar_texto(ui, estado->mensagem_temporaria,
		                  LARGURA_JANELA / 2 - 200, 50, ui->font_normal, cor_msg);
		estado->tempo_mensagem -= 0.016f;  // ~60 FPS
	}

	SDL_RenderPresent(ui->renderer);
}

void ui_renderizar_menu_principal(UIGrafica* ui, UIEstado* estado __attribute__((unused))) {
	SDL_Color cor_branca = {255, 255, 255, 255};

	// Título
	ui_desenhar_texto(ui, "TRUCO ESPANHOL", LARGURA_JANELA / 2 - 200, 100,
	                  ui->font_titulo, cor_branca);

	// Opções
	ui_desenhar_texto(ui, "Escolha uma opcao:", 100, 250, ui->font_normal, cor_branca);
}

void ui_renderizar_criar_sala(UIGrafica* ui, UIEstado* estado) {
	SDL_Color cor_branca = {255, 255, 255, 255};

	ui_desenhar_texto(ui, "CRIAR SALA", LARGURA_JANELA / 2 - 100, 100,
	                  ui->font_titulo, cor_branca);

	ui_desenhar_texto(ui, "Nome da sala:", 100, 250, ui->font_normal, cor_branca);

	// Campo de texto
	SDL_Rect campo = {100, 300, 400, 40};
	SDL_Color cor_campo = {255, 255, 255, 255};
	ui_desenhar_retangulo(ui, campo, cor_campo, false);

	ui_desenhar_texto(ui, estado->input_texto, 110, 310, ui->font_normal, cor_branca);
}

void ui_renderizar_listar_salas(UIGrafica* ui, UIEstado* estado) {
	SDL_Color cor_branca = {255, 255, 255, 255};

	ui_desenhar_texto(ui, "SALAS DISPONIVEIS", LARGURA_JANELA / 2 - 150, 50,
	                  ui->font_titulo, cor_branca);

	if (estado->num_salas == 0) {
		ui_desenhar_texto(ui, "Nenhuma sala disponivel", 100, 200,
		                  ui->font_normal, cor_branca);
	} else {
		for (int i = 0; i < estado->num_salas && i < 10; i++) {
			char texto[256];
			snprintf(texto, sizeof(texto), "ID: %u | %s | Jogadores: %u/2 %s",
			         estado->salas[i].id, estado->salas[i].nome,
			         estado->salas[i].num_jogadores,
			         estado->salas[i].em_partida ? "[EM JOGO]" : "");

			ui_desenhar_texto(ui, texto, 100, 150 + i * 40,
			                  ui->font_pequena, cor_branca);
		}
	}
}

void ui_renderizar_lobby(UIGrafica* ui, UIEstado* estado) {
	SDL_Color cor_branca = {255, 255, 255, 255};

	ui_desenhar_texto(ui, "LOBBY", LARGURA_JANELA / 2 - 50, 100,
	                  ui->font_titulo, cor_branca);

	char texto[128];
	snprintf(texto, sizeof(texto), "Sala ID: %u", estado->sala_id);
	ui_desenhar_texto(ui, texto, 100, 250, ui->font_normal, cor_branca);

	ui_desenhar_texto(ui, "Aguardando outro jogador...", 100, 300,
	                  ui->font_normal, cor_branca);
}

void ui_renderizar_jogo(UIGrafica* ui, UIEstado* estado) {
	SDL_Color cor_branca = {255, 255, 255, 255};
	SDL_Color cor_amarela = {255, 255, 0, 255};

	// Placar
	char placar[128];
	snprintf(placar, sizeof(placar), "VOCE: %d  x  %d :OPONENTE",
	         estado->estado_jogo.pontos_jogador1, estado->estado_jogo.pontos_jogador2);
	ui_desenhar_texto(ui, placar, LARGURA_JANELA / 2 - 150, 30,
	                  ui->font_normal, cor_branca);

	// Valor da rodada
	char valor[64];
	snprintf(valor, sizeof(valor), "Rodada vale: %d pontos", estado->estado_jogo.valor_rodada);
	ui_desenhar_texto(ui, valor, LARGURA_JANELA / 2 - 100, 70,
	                  ui->font_pequena, cor_amarela);

	// Indicador de mão
	const char* mao_texto = estado->estado_jogo.mao_jogador == 1 ? "[VOCE E MAO]" : "[OPONENTE E MAO]";
	ui_desenhar_texto(ui, mao_texto, LARGURA_JANELA / 2 - 80, 100,
	                  ui->font_pequena, cor_branca);

	// Vez
	if (estado->estado_jogo.vez_jogador == 1) {
		ui_desenhar_texto(ui, ">>> SUA VEZ <<<", LARGURA_JANELA / 2 - 80, 130,
		                  ui->font_normal, cor_amarela);
	} else {
		ui_desenhar_texto(ui, "Vez do oponente...", LARGURA_JANELA / 2 - 90, 130,
		                  ui->font_normal, cor_branca);
	}

	// Cartas do oponente (verso) - acima da mesa
	int oponente_y = 180;
	int oponente_x = LARGURA_JANELA / 2 - (LARGURA_CARTA + ESPACAMENTO_CARTA);
	for (int i = 0; i < 3; i++) {
		ui_desenhar_carta_verso(ui, oponente_x + i * (LARGURA_CARTA + ESPACAMENTO_CARTA), oponente_y);
	}

	// Cartas jogadas em TODAS as rodadas (3 rodadas, 2 cartas cada)
	// Layout: Cada rodada em uma coluna, J1 em cima, J2 embaixo
	int centro_y = ALTURA_JANELA / 2 - ALTURA_CARTA / 2;
	int mesa_y = centro_y + 10;
	int rodada_spacing = LARGURA_CARTA + 30;
	int inicio_x = LARGURA_JANELA / 2 - (rodada_spacing * 3) / 2 + 15;

	for (int rodada = 0; rodada < 3; rodada++) {
		int x = inicio_x + rodada * rodada_spacing;

		// Carta do oponente (em cima, perto das cartas de verso)
		if (estado->estado_jogo.cartas_jogadas_rodada[rodada * 2 + 1].numero != 0) {
			ui_desenhar_carta(ui, estado->estado_jogo.cartas_jogadas_rodada[rodada * 2 + 1],
			                  x, mesa_y, false, false);
		}

		// Sua carta (embaixo, perto da sua mão)
		if (estado->estado_jogo.cartas_jogadas_rodada[rodada * 2].numero != 0) {
			ui_desenhar_carta(ui, estado->estado_jogo.cartas_jogadas_rodada[rodada * 2],
			                  x, mesa_y + 80, false, false);  // 80px abaixo
		}
	}  // Suas cartas
	int mao_y = ALTURA_JANELA - ALTURA_CARTA - 50;
	int mao_x = LARGURA_JANELA / 2 - (estado->estado_jogo.num_cartas_mao * (LARGURA_CARTA + ESPACAMENTO_CARTA)) / 2;

	for (int i = 0; i < estado->estado_jogo.num_cartas_mao; i++) {
		int carta_x = mao_x + i * (LARGURA_CARTA + ESPACAMENTO_CARTA);
		bool selecionada = (ui->carta_selecionada == i);
		bool hover = ui->mouse_sobre_carta[i];

		ui_desenhar_carta(ui, estado->estado_jogo.cartas_mao[i],
		                  carta_x, mao_y, selecionada, hover);
	}
}

void ui_renderizar_fim_partida(UIGrafica* ui, UIEstado* estado) {
	SDL_Color cor_branca = {255, 255, 255, 255};
	SDL_Color cor_verde = {0, 255, 0, 255};
	SDL_Color cor_vermelha = {255, 0, 0, 255};

	ui_desenhar_texto(ui, "FIM DE PARTIDA!", LARGURA_JANELA / 2 - 150, 200,
	                  ui->font_titulo, cor_branca);

	if (estado->vencedor_partida == estado->meu_id) {
		ui_desenhar_texto(ui, "VOCE VENCEU!", LARGURA_JANELA / 2 - 100, 300,
		                  ui->font_normal, cor_verde);
	} else {
		ui_desenhar_texto(ui, "VOCE PERDEU!", LARGURA_JANELA / 2 - 100, 300,
		                  ui->font_normal, cor_vermelha);
	}
}

void ui_processar_evento(UIGrafica* ui, UIEstado* estado, SDL_Event* evento) {
	switch (evento->type) {
		case SDL_MOUSEMOTION:
			ui_atualizar_botoes(ui, evento->motion.x, evento->motion.y);

			// Verificar hover sobre cartas
			if (estado->tela_atual == TELA_JOGO) {
				int mao_y = ALTURA_JANELA - ALTURA_CARTA - 50;
				int mao_x = LARGURA_JANELA / 2 - (estado->estado_jogo.num_cartas_mao * (LARGURA_CARTA + ESPACAMENTO_CARTA)) / 2;

				for (int i = 0; i < estado->estado_jogo.num_cartas_mao; i++) {
					int carta_x = mao_x + i * (LARGURA_CARTA + ESPACAMENTO_CARTA);

					ui->mouse_sobre_carta[i] = (evento->motion.x >= carta_x &&
					                            evento->motion.x <= carta_x + LARGURA_CARTA &&
					                            evento->motion.y >= mao_y &&
					                            evento->motion.y <= mao_y + ALTURA_CARTA);
				}
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (evento->button.button == SDL_BUTTON_LEFT) {
				// IMPORTANTE: Processar botões PRIMEIRO (prioridade maior)
				Botao* botao = ui_obter_botao_clicado(ui, evento->button.x, evento->button.y);
				if (botao && botao->callback) {
					botao->callback(botao->callback_data);
					break;  // NÃO processar cartas se clicou em botão
				}

				// Se não clicou em botão, verificar clique em carta
				if (estado->tela_atual == TELA_JOGO) {
					int mao_y = ALTURA_JANELA - ALTURA_CARTA - 50;
					int mao_x = LARGURA_JANELA / 2 - (estado->estado_jogo.num_cartas_mao * (LARGURA_CARTA + ESPACAMENTO_CARTA)) / 2;

					for (int i = 0; i < estado->estado_jogo.num_cartas_mao; i++) {
						int carta_x = mao_x + i * (LARGURA_CARTA + ESPACAMENTO_CARTA);

						if (evento->button.x >= carta_x &&
						    evento->button.x <= carta_x + LARGURA_CARTA &&
						    evento->button.y >= mao_y &&
						    evento->button.y <= mao_y + ALTURA_CARTA) {
							ui->carta_selecionada = i;
							printf(">>> Carta %d SELECIONADA <<<\n", i);
							break;
						}
					}
				}
			}
			break;

		case SDL_KEYDOWN:
			if (estado->tela_atual == TELA_CRIAR_SALA) {
				if (evento->key.keysym.sym == SDLK_BACKSPACE && strlen(estado->input_texto) > 0) {
					estado->input_texto[strlen(estado->input_texto) - 1] = '\0';
				}
			}
			break;

		case SDL_TEXTINPUT:
			if (estado->tela_atual == TELA_CRIAR_SALA) {
				if (strlen(estado->input_texto) < sizeof(estado->input_texto) - 1) {
					strcat(estado->input_texto, evento->text.text);
				}
			}
			break;
	}
}
