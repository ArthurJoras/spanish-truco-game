#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define PORTA_PADRAO 8888
#define SERVER_IP_PADRAO "127.0.0.1"
#define SERVER_PORTA_PADRAO PORTA_PADRAO

// Enums para identificação das cartas
typedef enum {
	NAIPE_ESPADAS = 0,
	NAIPE_PAUS = 1,
	NAIPE_COPAS = 2,
	NAIPE_OUROS = 3
} Naipe;

typedef enum {
	NUMERO_AS = 1,
	NUMERO_2 = 2,
	NUMERO_3 = 3,
	NUMERO_4 = 4,
	NUMERO_5 = 5,
	NUMERO_6 = 6,
	NUMERO_7 = 7,
	NUMERO_10 = 10,
	NUMERO_11 = 11,
	NUMERO_12 = 12
} NumeroCarta;

// Estrutura de uma carta
typedef struct {
	Naipe naipe;
	NumeroCarta numero;
} Carta;

// Tipos de mensagem do protocolo
typedef enum {
	MSG_CONECTAR = 0,
	MSG_CRIAR_SALA = 1,
	MSG_ENTRAR_SALA = 2,
	MSG_LISTAR_SALAS = 3,
	MSG_INICIAR_PARTIDA = 4,
	MSG_JOGAR_CARTA = 5,
	MSG_TRUCO = 6,
	MSG_RESPOSTA_TRUCO = 7,
	MSG_ENVIDO = 8,
	MSG_RESPOSTA_ENVIDO = 9,
	MSG_FLOR = 10,
	MSG_RESPOSTA_FLOR = 11,
	MSG_ESTADO_JOGO = 12,
	MSG_RESULTADO_RODADA = 13,
	MSG_FIM_PARTIDA = 14,
	MSG_ERRO = 15,
	MSG_DESCONECTAR = 16,
	MSG_IR_BARALHO = 17,
	MSG_SAIR_SALA = 18
} TipoMensagem;

// Respostas ao truco
typedef enum {
	RESPOSTA_QUERO = 0,
	RESPOSTA_NAO_QUERO = 1,
	RESPOSTA_RETRUCO = 2,
	RESPOSTA_VALE_QUATRO = 3
} RespostaTruco;

// Respostas ao envido
typedef enum {
	ENVIDO_QUERO = 0,
	ENVIDO_NAO_QUERO = 1,
	ENVIDO_REAL_ENVIDO = 2,
	ENVIDO_FALTA_ENVIDO = 3
} RespostaEnvido;

// Respostas à flor
typedef enum {
	FLOR_QUERO = 0,
	FLOR_NAO_QUERO = 1,
	FLOR_CONTRAFLOR = 2,
	FLOR_CONTRAFLOR_RESTO = 3
} RespostaFlor;

// Estrutura de mensagem genérica
typedef struct {
	TipoMensagem tipo;
	uint32_t sala_id;
	uint32_t jogador_id;
	uint8_t dados[512];
	uint32_t tamanho_dados;
} Mensagem;

// Informações de uma sala
typedef struct {
	uint32_t id;
	char nome[64];
	uint8_t num_jogadores;
	uint8_t max_jogadores;
	uint8_t em_partida;
} InfoSala;

// Estado do jogo para enviar ao cliente
typedef struct {
	uint8_t pontos_jogador1;
	uint8_t pontos_jogador2;
	uint8_t rodada_atual;
	uint8_t num_cartas_mao;
	Carta cartas_mao[3];
	Carta cartas_jogadas_rodada[6];  // 3 rodadas x 2 jogadores = 6 cartas
	uint8_t mao_jogador;
	uint8_t vez_jogador;
	uint8_t valor_rodada;
	uint8_t valor_envido;
	uint8_t valor_flor;
	uint8_t pode_cantar_truco;
	uint8_t pode_cantar_envido;
	uint8_t pode_cantar_flor;
	uint8_t aguardando_resposta;
} EstadoJogo;

// Constantes
#define MAX_CLIENTES 100
#define MAX_SALAS 50
#define PONTOS_VITORIA 15
#define CARTAS_POR_JOGADOR 3

// Funções auxiliares
const char* naipe_para_string(Naipe naipe);
const char* numero_para_string(NumeroCarta numero);
void imprimir_carta(Carta carta);

#endif  // COMMON_H
