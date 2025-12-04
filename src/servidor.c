#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "game_logic.h"

// Estrutura de uma sala
typedef struct {
	uint32_t id;
	char nome[64];
	int jogador1_socket;
	int jogador2_socket;
	uint32_t jogador1_id;
	uint32_t jogador2_id;
	bool ativa;
	bool em_partida;
	Jogo jogo;
	pthread_mutex_t mutex;
} Sala;

// Estrutura de cliente conectado
typedef struct {
	int socket;
	uint32_t id;
	uint32_t sala_id;
	bool ativo;
} Cliente;

// Variáveis globais
static Sala salas[MAX_SALAS];
static Cliente clientes[MAX_CLIENTES];
static pthread_mutex_t salas_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t clientes_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t proximo_cliente_id = 1;
static uint32_t proxima_sala_id = 1;

// Funções auxiliares
void inicializar_servidor();
Cliente* obter_cliente_por_socket(int socket);
Sala* obter_sala_por_id(uint32_t sala_id);
Sala* criar_sala(const char* nome, int criador_socket, uint32_t criador_id);
bool entrar_sala(uint32_t sala_id, int socket, uint32_t cliente_id);
void remover_cliente_da_sala(Cliente* cliente);
void enviar_mensagem(int socket, Mensagem* msg);
bool receber_mensagem(int socket, Mensagem* msg);
void broadcast_sala(Sala* sala, Mensagem* msg, int exceto_socket);
void* thread_cliente(void* arg);
void processar_mensagem(Cliente* cliente, Mensagem* msg);

// Implementação
void inicializar_servidor() {
	memset(salas, 0, sizeof(salas));
	memset(clientes, 0, sizeof(clientes));

	for (int i = 0; i < MAX_SALAS; i++) {
		pthread_mutex_init(&salas[i].mutex, NULL);
	}
}

Cliente* obter_cliente_por_socket(int socket) {
	pthread_mutex_lock(&clientes_mutex);
	for (int i = 0; i < MAX_CLIENTES; i++) {
		if (clientes[i].ativo && clientes[i].socket == socket) {
			pthread_mutex_unlock(&clientes_mutex);
			return &clientes[i];
		}
	}
	pthread_mutex_unlock(&clientes_mutex);
	return NULL;
}

Sala* obter_sala_por_id(uint32_t sala_id) {
	pthread_mutex_lock(&salas_mutex);
	for (int i = 0; i < MAX_SALAS; i++) {
		if (salas[i].ativa && salas[i].id == sala_id) {
			pthread_mutex_unlock(&salas_mutex);
			return &salas[i];
		}
	}
	pthread_mutex_unlock(&salas_mutex);
	return NULL;
}

Sala* criar_sala(const char* nome, int criador_socket, uint32_t criador_id) {
	pthread_mutex_lock(&salas_mutex);

	for (int i = 0; i < MAX_SALAS; i++) {
		if (!salas[i].ativa) {
			salas[i].id = proxima_sala_id++;
			strncpy(salas[i].nome, nome, sizeof(salas[i].nome) - 1);
			salas[i].jogador1_socket = criador_socket;
			salas[i].jogador1_id = criador_id;
			salas[i].jogador2_socket = -1;
			salas[i].jogador2_id = 0;
			salas[i].ativa = true;
			salas[i].em_partida = false;

			pthread_mutex_unlock(&salas_mutex);
			return &salas[i];
		}
	}

	pthread_mutex_unlock(&salas_mutex);
	return NULL;
}

bool entrar_sala(uint32_t sala_id, int socket, uint32_t cliente_id) {
	Sala* sala = obter_sala_por_id(sala_id);
	if (!sala) return false;

	pthread_mutex_lock(&sala->mutex);

	// Prioriza slot jogador2, mas aceita jogador1 se vazio
	if (sala->jogador2_socket == -1) {
		sala->jogador2_socket = socket;
		sala->jogador2_id = cliente_id;
		pthread_mutex_unlock(&sala->mutex);
		return true;
	} else if (sala->jogador1_socket == -1) {
		// Se jogador2 está ocupado mas jogador1 vazio (host saiu), aceita como jogador1
		sala->jogador1_socket = socket;
		sala->jogador1_id = cliente_id;
		pthread_mutex_unlock(&sala->mutex);
		return true;
	}

	pthread_mutex_unlock(&sala->mutex);
	return false;
}

void remover_cliente_da_sala(Cliente* cliente) {
	if (cliente->sala_id == 0) return;

	Sala* sala = obter_sala_por_id(cliente->sala_id);
	if (!sala) return;

	pthread_mutex_lock(&sala->mutex);

	int jogador_saiu = 0;
	int socket_restante = -1;

	if (sala->jogador1_socket == cliente->socket) {
		sala->jogador1_socket = -1;
		sala->jogador1_id = 0;
		jogador_saiu = 1;
		socket_restante = sala->jogador2_socket;
	} else if (sala->jogador2_socket == cliente->socket) {
		sala->jogador2_socket = -1;
		sala->jogador2_id = 0;
		jogador_saiu = 2;
		socket_restante = sala->jogador1_socket;
	}

	// Notifica o jogador restante (se houver)
	if (socket_restante != -1 && jogador_saiu != 0) {
		Mensagem notif;
		memset(&notif, 0, sizeof(Mensagem));
		notif.tipo = MSG_ENTRAR_SALA;  // Reutiliza mensagem para atualizar contagem
		notif.sala_id = sala->id;
		notif.jogador_id = 0;  // 0 indica que alguém saiu
		enviar_mensagem(socket_restante, &notif);
	}

	// Se ficou vazia, desativa a sala
	if (sala->jogador1_socket == -1 && sala->jogador2_socket == -1) {
		sala->ativa = false;
		printf("Sala %u destruída (todos saíram)\n", sala->id);
	}

	pthread_mutex_unlock(&sala->mutex);
	cliente->sala_id = 0;
}

void enviar_mensagem(int socket, Mensagem* msg) {
	send(socket, msg, sizeof(Mensagem), 0);
}

bool receber_mensagem(int socket, Mensagem* msg) {
	int bytes = recv(socket, msg, sizeof(Mensagem), 0);
	return bytes == sizeof(Mensagem);
}

void broadcast_sala(Sala* sala, Mensagem* msg, int exceto_socket) {
	// NOTA: Caller deve já possuir sala->mutex
	if (sala->jogador1_socket != -1 && sala->jogador1_socket != exceto_socket) {
		enviar_mensagem(sala->jogador1_socket, msg);
	}
	if (sala->jogador2_socket != -1 && sala->jogador2_socket != exceto_socket) {
		enviar_mensagem(sala->jogador2_socket, msg);
	}
}

void processar_mensagem(Cliente* cliente, Mensagem* msg) {
	Mensagem resposta;
	memset(&resposta, 0, sizeof(Mensagem));

	switch (msg->tipo) {
		case MSG_CRIAR_SALA: {
			char nome_sala[64];
			memcpy(nome_sala, msg->dados, sizeof(nome_sala));

			Sala* sala = criar_sala(nome_sala, cliente->socket, cliente->id);
			if (sala) {
				cliente->sala_id = sala->id;
				resposta.tipo = MSG_CRIAR_SALA;
				resposta.sala_id = sala->id;
				resposta.jogador_id = cliente->id;
				enviar_mensagem(cliente->socket, &resposta);

				printf("Cliente %u criou sala %u: %s\n", cliente->id, sala->id, nome_sala);
			} else {
				resposta.tipo = MSG_ERRO;
				enviar_mensagem(cliente->socket, &resposta);
			}
			break;
		}

		case MSG_LISTAR_SALAS: {
			pthread_mutex_lock(&salas_mutex);

			int num_salas = 0;
			InfoSala info_salas[MAX_SALAS];

			for (int i = 0; i < MAX_SALAS; i++) {
				if (salas[i].ativa) {
					info_salas[num_salas].id = salas[i].id;
					strncpy(info_salas[num_salas].nome, salas[i].nome, 64);
					info_salas[num_salas].num_jogadores =
					    (salas[i].jogador1_socket != -1 ? 1 : 0) +
					    (salas[i].jogador2_socket != -1 ? 1 : 0);
					info_salas[num_salas].max_jogadores = 2;
					info_salas[num_salas].em_partida = salas[i].em_partida;
					num_salas++;
				}
			}

			pthread_mutex_unlock(&salas_mutex);

			resposta.tipo = MSG_LISTAR_SALAS;
			resposta.tamanho_dados = num_salas * sizeof(InfoSala);
			memcpy(resposta.dados, info_salas, resposta.tamanho_dados);
			enviar_mensagem(cliente->socket, &resposta);
			break;
		}

		case MSG_ENTRAR_SALA: {
			uint32_t sala_id;
			memcpy(&sala_id, msg->dados, sizeof(uint32_t));

			// Verifica se já está na sala
			if (cliente->sala_id == sala_id) {
				resposta.tipo = MSG_ERRO;
				const char* msg_erro = "Voce ja esta nesta sala";
				memcpy(resposta.dados, msg_erro, strlen(msg_erro) + 1);
				enviar_mensagem(cliente->socket, &resposta);
				break;
			}

			if (entrar_sala(sala_id, cliente->socket, cliente->id)) {
				cliente->sala_id = sala_id;
				resposta.tipo = MSG_ENTRAR_SALA;
				resposta.sala_id = sala_id;
				resposta.jogador_id = cliente->id;
				enviar_mensagem(cliente->socket, &resposta);

				// Notifica o outro jogador
				Sala* sala = obter_sala_por_id(sala_id);
				if (sala) {
					pthread_mutex_lock(&sala->mutex);
					Mensagem notif;
					memset(&notif, 0, sizeof(Mensagem));
					notif.tipo = MSG_ENTRAR_SALA;
					notif.sala_id = sala_id;
					notif.jogador_id = cliente->id;  // ID do jogador que entrou
					broadcast_sala(sala, &notif, cliente->socket);
					pthread_mutex_unlock(&sala->mutex);
				}
				printf("Cliente %u entrou na sala %u\n", cliente->id, sala_id);
			} else {
				resposta.tipo = MSG_ERRO;
				const char* msg_erro = "Sala cheia";
				memcpy(resposta.dados, msg_erro, strlen(msg_erro) + 1);
				enviar_mensagem(cliente->socket, &resposta);
			}
			break;
		}

		case MSG_SAIR_SALA: {
			printf("Cliente %u saindo da sala %u\n", cliente->id, cliente->sala_id);
			remover_cliente_da_sala(cliente);  // Já reseta cliente->sala_id = 0
			// Envia confirmação (jogador_id=0 indica que é resposta de saída)
			resposta.tipo = MSG_CONECTAR;
			resposta.jogador_id = 0;
			enviar_mensagem(cliente->socket, &resposta);
			break;
		}
		case MSG_INICIAR_PARTIDA: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->jogador1_socket != -1 && sala->jogador2_socket != -1) {
				pthread_mutex_lock(&sala->mutex);

				inicializar_jogo(&sala->jogo, sala->id);
				sala->jogo.jogador1.id = sala->jogador1_id;
				sala->jogo.jogador2.id = sala->jogador2_id;
				inicializar_baralho(&sala->jogo.baralho);
				distribuir_cartas(&sala->jogo);
				sala->em_partida = true;

				pthread_mutex_unlock(&sala->mutex);

				// Envia estado inicial para ambos jogadores
				resposta.tipo = MSG_ESTADO_JOGO;
				resposta.sala_id = sala->id;

				EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
				resposta.jogador_id = sala->jogador1_id;
				memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador1_socket, &resposta);

				EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
				resposta.jogador_id = sala->jogador2_id;
				memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador2_socket, &resposta);

				printf("Partida iniciada na sala %u\n", sala->id);
			}
			break;
		}

		case MSG_JOGAR_CARTA: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;
				int indice_carta;
				memcpy(&indice_carta, msg->dados, sizeof(int));

				if (jogar_carta(&sala->jogo, jogador, indice_carta)) {
					// Envia estado atualizado para ambos
					resposta.tipo = MSG_ESTADO_JOGO;
					resposta.sala_id = sala->id;

					EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
					resposta.jogador_id = sala->jogador1_id;
					memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador1_socket, &resposta);

					EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
					resposta.jogador_id = sala->jogador2_id;
					memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador2_socket, &resposta);

					// Verifica se a mão terminou (3 rodadas completas ou 2 vitórias)
					if (sala->jogo.rodada_atual >= 3 && !sala->jogo.partida_finalizada) {
						printf("Mão finalizada! Iniciando nova mão...\n");
						nova_mao(&sala->jogo);

						// Envia novo estado após nova mão
						resposta.tipo = MSG_ESTADO_JOGO;

						estado1 = obter_estado_jogo(&sala->jogo, 1);
						resposta.jogador_id = sala->jogador1_id;
						memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
						enviar_mensagem(sala->jogador1_socket, &resposta);

						estado2 = obter_estado_jogo(&sala->jogo, 2);
						resposta.jogador_id = sala->jogador2_id;
						memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
						enviar_mensagem(sala->jogador2_socket, &resposta);
					}

					// Verifica se a partida terminou
					if (sala->jogo.partida_finalizada) {
						Mensagem fim;
						memset(&fim, 0, sizeof(Mensagem));
						fim.tipo = MSG_FIM_PARTIDA;
						fim.sala_id = sala->id;
						// Envia ID do cliente vencedor (não o número do jogador)
						uint32_t id_vencedor = (sala->jogo.vencedor_partida == 1) ? sala->jogador1_id : sala->jogador2_id;
						memcpy(fim.dados, &id_vencedor, sizeof(uint32_t));
						broadcast_sala(sala, &fim, -1);
						sala->em_partida = false;
						sala->ativa = false;  // Destrói a sala após fim da partida
						printf("Partida finalizada na sala %u - Vencedor: Jogador %d\n",
						       sala->id, sala->jogo.vencedor_partida);
						printf("Sala %u destruída\n", sala->id);
					}
				}

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		case MSG_TRUCO: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;

				if (cantar_truco(&sala->jogo, jogador)) {
					// Primeiro envia notificação MSG_TRUCO
					resposta.tipo = MSG_TRUCO;
					resposta.sala_id = sala->id;
					resposta.jogador_id = cliente->id;
					broadcast_sala(sala, &resposta, -1);

					// Depois envia estado atualizado (com aguardando_resposta=1)
					resposta.tipo = MSG_ESTADO_JOGO;
					resposta.sala_id = sala->id;

					EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
					resposta.jogador_id = sala->jogador1_id;
					memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador1_socket, &resposta);

					EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
					resposta.jogador_id = sala->jogador2_id;
					memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador2_socket, &resposta);
				}

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}
		case MSG_RESPOSTA_TRUCO: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;
				RespostaTruco resp;
				memcpy(&resp, msg->dados, sizeof(RespostaTruco));

				responder_truco(&sala->jogo, jogador, resp);

				// Envia estado atualizado
				resposta.tipo = MSG_ESTADO_JOGO;
				resposta.sala_id = sala->id;

				EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
				resposta.jogador_id = sala->jogador1_id;
				memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador1_socket, &resposta);

				EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
				resposta.jogador_id = sala->jogador2_id;
				memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador2_socket, &resposta);

				// Verifica se a partida terminou
				if (sala->jogo.partida_finalizada) {
					Mensagem fim;
					memset(&fim, 0, sizeof(Mensagem));
					fim.tipo = MSG_FIM_PARTIDA;
					fim.sala_id = sala->id;
					// Envia ID do cliente vencedor (não o número do jogador)
					uint32_t id_vencedor = (sala->jogo.vencedor_partida == 1) ? sala->jogador1_id : sala->jogador2_id;
					memcpy(fim.dados, &id_vencedor, sizeof(uint32_t));
					broadcast_sala(sala, &fim, -1);
					sala->em_partida = false;
					printf("Partida finalizada na sala %u - Vencedor: Jogador %d\n",
					       sala->id, sala->jogo.vencedor_partida);
				}

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		case MSG_ENVIDO: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;

				if (cantar_envido(&sala->jogo, jogador)) {
					// Primeiro envia notificação MSG_ENVIDO
					resposta.tipo = MSG_ENVIDO;
					resposta.sala_id = sala->id;
					resposta.jogador_id = cliente->id;
					broadcast_sala(sala, &resposta, -1);

					// Depois envia estado atualizado (com aguardando_resposta=1)
					resposta.tipo = MSG_ESTADO_JOGO;
					resposta.sala_id = sala->id;

					EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
					resposta.jogador_id = sala->jogador1_id;
					memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador1_socket, &resposta);

					EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
					resposta.jogador_id = sala->jogador2_id;
					memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador2_socket, &resposta);
				}

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		case MSG_RESPOSTA_ENVIDO: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;
				RespostaEnvido resp;
				memcpy(&resp, msg->dados, sizeof(RespostaEnvido));

				responder_envido(&sala->jogo, jogador, resp);

				// Envia estado atualizado
				resposta.tipo = MSG_ESTADO_JOGO;
				resposta.sala_id = sala->id;

				EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
				resposta.jogador_id = sala->jogador1_id;
				memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador1_socket, &resposta);

				EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
				resposta.jogador_id = sala->jogador2_id;
				memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador2_socket, &resposta);

				// Verifica se a partida terminou
				if (sala->jogo.partida_finalizada) {
					Mensagem fim;
					memset(&fim, 0, sizeof(Mensagem));
					fim.tipo = MSG_FIM_PARTIDA;
					fim.sala_id = sala->id;
					// Envia ID do cliente vencedor (não o número do jogador)
					uint32_t id_vencedor = (sala->jogo.vencedor_partida == 1) ? sala->jogador1_id : sala->jogador2_id;
					memcpy(fim.dados, &id_vencedor, sizeof(uint32_t));
					broadcast_sala(sala, &fim, -1);
					sala->em_partida = false;
					printf("Partida finalizada na sala %u - Vencedor: Jogador %d\n",
					       sala->id, sala->jogo.vencedor_partida);
				}

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		case MSG_RESPOSTA_FLOR: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;
				RespostaFlor resp;
				memcpy(&resp, msg->dados, sizeof(RespostaFlor));

				responder_flor(&sala->jogo, jogador, resp);

				// Envia estado atualizado
				resposta.tipo = MSG_ESTADO_JOGO;
				resposta.sala_id = sala->id;

				EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
				resposta.jogador_id = sala->jogador1_id;
				memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador1_socket, &resposta);

				EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
				resposta.jogador_id = sala->jogador2_id;
				memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador2_socket, &resposta);

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		case MSG_IR_BARALHO: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;
				ir_baralho(&sala->jogo, jogador);  // Envia estado atualizado
				resposta.tipo = MSG_ESTADO_JOGO;
				resposta.sala_id = sala->id;

				EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
				resposta.jogador_id = sala->jogador1_id;
				memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador1_socket, &resposta);

				EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
				resposta.jogador_id = sala->jogador2_id;
				memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
				enviar_mensagem(sala->jogador2_socket, &resposta);

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		case MSG_FLOR: {
			Sala* sala = obter_sala_por_id(cliente->sala_id);
			if (sala && sala->em_partida) {
				pthread_mutex_lock(&sala->mutex);

				int jogador = (cliente->socket == sala->jogador1_socket) ? 1 : 2;

				if (cantar_flor(&sala->jogo, jogador)) {
					// Primeiro envia notificação MSG_FLOR
					resposta.tipo = MSG_FLOR;
					resposta.sala_id = sala->id;
					resposta.jogador_id = cliente->id;
					broadcast_sala(sala, &resposta, -1);

					// Depois envia estado atualizado (com aguardando_resposta=1)
					resposta.tipo = MSG_ESTADO_JOGO;
					resposta.sala_id = sala->id;

					EstadoJogo estado1 = obter_estado_jogo(&sala->jogo, 1);
					resposta.jogador_id = sala->jogador1_id;
					memcpy(resposta.dados, &estado1, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador1_socket, &resposta);

					EstadoJogo estado2 = obter_estado_jogo(&sala->jogo, 2);
					resposta.jogador_id = sala->jogador2_id;
					memcpy(resposta.dados, &estado2, sizeof(EstadoJogo));
					enviar_mensagem(sala->jogador2_socket, &resposta);
				}

				pthread_mutex_unlock(&sala->mutex);
			}
			break;
		}

		default:
			break;
	}
}

void* thread_cliente(void* arg) {
	int socket = *(int*)arg;
	free(arg);

	// Registra cliente
	pthread_mutex_lock(&clientes_mutex);
	Cliente* cliente = NULL;
	for (int i = 0; i < MAX_CLIENTES; i++) {
		if (!clientes[i].ativo) {
			clientes[i].socket = socket;
			clientes[i].id = proximo_cliente_id++;
			clientes[i].sala_id = 0;
			clientes[i].ativo = true;
			cliente = &clientes[i];
			break;
		}
	}
	pthread_mutex_unlock(&clientes_mutex);

	if (!cliente) {
		close(socket);
		return NULL;
	}

	// Envia ID do cliente
	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_CONECTAR;
	msg.jogador_id = cliente->id;
	enviar_mensagem(socket, &msg);

	// Loop de recebimento de mensagens
	while (receber_mensagem(socket, &msg)) {
		processar_mensagem(cliente, &msg);
	}

	// Cliente desconectou
	remover_cliente_da_sala(cliente);

	pthread_mutex_lock(&clientes_mutex);
	cliente->ativo = false;
	pthread_mutex_unlock(&clientes_mutex);

	close(socket);
	return NULL;
}

int main(int argc, char* argv[]) {
	int porta = PORTA_PADRAO;

	if (argc > 1) {
		porta = atoi(argv[1]);
	}

	printf("Iniciando servidor de Truco na porta %d...\n", porta);

	inicializar_servidor();

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("Erro ao criar socket");
		return 1;
	}

	int opt = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(porta);

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Erro ao fazer bind");
		close(server_socket);
		return 1;
	}

	if (listen(server_socket, 10) < 0) {
		perror("Erro ao escutar");
		close(server_socket);
		return 1;
	}

	printf("Servidor rodando! Aguardando conexões...\n");

	while (1) {
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
		if (client_socket < 0) {
			perror("Erro ao aceitar conexão");
			continue;
		}

		int* socket_ptr = malloc(sizeof(int));
		*socket_ptr = client_socket;

		pthread_t thread;
		pthread_create(&thread, NULL, thread_cliente, socket_ptr);
		pthread_detach(thread);
	}

	close(server_socket);
	return 0;
}
