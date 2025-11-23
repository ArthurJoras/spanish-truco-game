#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>

#include "common.h"

// Estrutura do baralho
typedef struct {
	Carta cartas[40];
	int topo;
} Baralho;

// Estrutura de um jogador
typedef struct {
	uint32_t id;
	char nome[64];
	Carta mao[3];
	int num_cartas;
	uint8_t pontos_envido;
	bool tem_flor;
} Jogador;

// Estrutura de uma rodada
typedef struct {
	Carta carta_jogador1;
	Carta carta_jogador2;
	bool jogador1_jogou;
	bool jogador2_jogou;
	int vencedor;  // 0 = empate, 1 = jogador1, 2 = jogador2
} Rodada;

// Estrutura do jogo
typedef struct {
	uint32_t sala_id;
	Jogador jogador1;
	Jogador jogador2;
	Baralho baralho;
	Rodada rodadas[3];
	int rodada_atual;
	int mao_jogador;  // Quem é mão (1 ou 2)
	int vez_jogador;  // De quem é a vez (1 ou 2)
	int pontos_jogador1;
	int pontos_jogador2;
	int valor_rodada;  // Valor atual da rodada (1, 2, 3, 4)
	int valor_envido;  // Valor atual do envido (2=Envido, 3=Real, 4=Falta)
	int valor_flor;    // Valor atual da flor (3=Flor, 4=Contraflor, 5=Contraflor Resto)
	bool truco_cantado;
	bool envido_cantado;
	bool flor_cantada;
	bool aguardando_resposta_truco;
	bool aguardando_resposta_envido;
	bool aguardando_resposta_flor;
	int jogador_cantou_truco;
	int jogador_cantou_envido;
	int jogador_cantou_flor;
	int ultimo_a_aumentar_truco;   // Último jogador a aumentar aposta (Retruco/Vale4)
	int ultimo_a_aumentar_envido;  // Último jogador a aumentar aposta (Real/Falta Envido)
	bool partida_finalizada;
	int vencedor_partida;
} Jogo;

// Funções do baralho
void inicializar_baralho(Baralho* baralho);
void embaralhar(Baralho* baralho);
Carta pegar_carta(Baralho* baralho);

// Funções do jogo
void inicializar_jogo(Jogo* jogo, uint32_t sala_id);
void distribuir_cartas(Jogo* jogo);
void nova_mao(Jogo* jogo);

// Funções de comparação de cartas
int obter_valor_carta_truco(Carta carta);
int comparar_cartas_truco(Carta c1, Carta c2);
int calcular_pontos_envido(Jogador* jogador);
bool verificar_flor(Jogador* jogador);

// Funções de jogada
bool pode_jogar_carta(Jogo* jogo, int jogador, int indice_carta);
bool jogar_carta(Jogo* jogo, int jogador, int indice_carta);
void resolver_rodada(Jogo* jogo);
void finalizar_mao(Jogo* jogo);

// Funções de cantos
bool pode_cantar_truco(Jogo* jogo, int jogador);
bool cantar_truco(Jogo* jogo, int jogador);
void responder_truco(Jogo* jogo, int jogador, RespostaTruco resposta);

bool pode_cantar_envido(Jogo* jogo, int jogador);
bool cantar_envido(Jogo* jogo, int jogador);
void responder_envido(Jogo* jogo, int jogador, RespostaEnvido resposta);

bool pode_cantar_flor(Jogo* jogo, int jogador);
bool cantar_flor(Jogo* jogo, int jogador);
void responder_flor(Jogo* jogo, int jogador, RespostaFlor resposta);

bool pode_ir_baralho(Jogo* jogo, int jogador);
void ir_baralho(Jogo* jogo, int jogador);

// Função para verificar fim de partida
bool verificar_fim_partida(Jogo* jogo);

// Função para obter estado do jogo para um jogador
EstadoJogo obter_estado_jogo(Jogo* jogo, int jogador);

#endif  // GAME_LOGIC_H
