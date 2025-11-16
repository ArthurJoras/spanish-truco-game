#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

// Estrutura do cliente
typedef struct {
	int socket;
	uint32_t id;
	uint32_t sala_id;
	bool conectado;
	bool em_partida;
	EstadoJogo estado;
	pthread_mutex_t mutex;
} ClienteState;

static ClienteState cliente_state;
static pthread_t thread_recebimento;

// Funções de interface
void limpar_tela();
void exibir_menu_principal();
void exibir_salas(InfoSala* salas, int num_salas);
void exibir_estado_jogo();
void exibir_cartas_mao();

// Funções de comunicação
bool conectar_servidor(const char* ip, int porta);
void desconectar_servidor();
bool enviar_mensagem(Mensagem* msg);
void* thread_receber_mensagens(void* arg);
void processar_mensagem_recebida(Mensagem* msg);

// Funções de ação
void criar_sala();
void listar_salas();
void entrar_sala();
void iniciar_partida();
void jogar_turno();
void menu_cantos();

// Implementação
void limpar_tela() {
	printf("\033[2J\033[H");
}

void exibir_menu_principal() {
	limpar_tela();
	printf("========================================\n");
	printf("         TRUCO ESPANHOL v1.0           \n");
	printf("========================================\n");
	printf("\n");

	if (cliente_state.sala_id > 0) {
		printf("Sala atual: %u\n", cliente_state.sala_id);
		if (cliente_state.em_partida) {
			printf("Pontos: Você %d x %d Oponente\n",
			       cliente_state.estado.pontos_jogador1,
			       cliente_state.estado.pontos_jogador2);
			printf("Valor da rodada: %d pontos\n", cliente_state.estado.valor_rodada);
		}
		printf("\n");
	}

	printf("1 - Criar sala\n");
	printf("2 - Listar salas\n");
	printf("3 - Entrar em sala\n");

	if (cliente_state.sala_id > 0 && !cliente_state.em_partida) {
		printf("4 - Iniciar partida\n");
	}

	if (cliente_state.em_partida) {
		printf("5 - Jogar turno\n");
		printf("6 - Cantos (Truco/Envido/Flor)\n");
	}

	printf("0 - Sair\n");
	printf("\nEscolha uma opção: ");
}

void exibir_salas(InfoSala* salas, int num_salas) {
	limpar_tela();
	printf("========================================\n");
	printf("           SALAS DISPONÍVEIS           \n");
	printf("========================================\n\n");

	if (num_salas == 0) {
		printf("Nenhuma sala disponível.\n");
	} else {
		for (int i = 0; i < num_salas; i++) {
			printf("ID: %u | Nome: %s | Jogadores: %u/%u | %s\n",
			       salas[i].id,
			       salas[i].nome,
			       salas[i].num_jogadores,
			       salas[i].max_jogadores,
			       salas[i].em_partida ? "EM PARTIDA" : "AGUARDANDO");
		}
	}

	printf("\nPressione ENTER para voltar...");
	getchar();
}

void exibir_cartas_mao() {
	printf("\n========== SUAS CARTAS ==========\n");

	for (int i = 0; i < cliente_state.estado.num_cartas_mao; i++) {
		printf("%d. ", i + 1);
		imprimir_carta(cliente_state.estado.cartas_mao[i]);
		printf("\n");
	}

	printf("================================\n");
}

void exibir_estado_jogo() {
	limpar_tela();
	printf("========================================\n");
	printf("              JOGO EM ANDAMENTO         \n");
	printf("========================================\n\n");

	printf("Pontos: Você %d x %d Oponente\n",
	       cliente_state.estado.pontos_jogador1,
	       cliente_state.estado.pontos_jogador2);
	printf("Rodada: %d/3\n", cliente_state.estado.rodada_atual + 1);
	printf("Valor da rodada: %d pontos\n", cliente_state.estado.valor_rodada);
	printf("Você é %s\n", cliente_state.estado.mao_jogador == 1 ? "MÃO" : "PÉ");

	if (cliente_state.estado.vez_jogador == 1) {
		printf(">>> SUA VEZ DE JOGAR <<<\n");
	} else {
		printf("Aguardando jogada do oponente...\n");
	}

	exibir_cartas_mao();

	if (cliente_state.estado.aguardando_resposta) {
		printf("\n!!! AGUARDANDO SUA RESPOSTA A UM CANTO !!!\n");
	}
}

bool conectar_servidor(const char* ip, int porta) {
	cliente_state.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (cliente_state.socket < 0) {
		perror("Erro ao criar socket");
		return false;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porta);

	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
		perror("Endereço inválido");
		close(cliente_state.socket);
		return false;
	}

	if (connect(cliente_state.socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Erro ao conectar");
		close(cliente_state.socket);
		return false;
	}

	cliente_state.conectado = true;
	pthread_mutex_init(&cliente_state.mutex, NULL);

	// Inicia thread de recebimento
	pthread_create(&thread_recebimento, NULL, thread_receber_mensagens, NULL);

	return true;
}

void desconectar_servidor() {
	if (cliente_state.conectado) {
		Mensagem msg;
		memset(&msg, 0, sizeof(Mensagem));
		msg.tipo = MSG_DESCONECTAR;
		enviar_mensagem(&msg);

		cliente_state.conectado = false;
		close(cliente_state.socket);
		pthread_cancel(thread_recebimento);
	}
}

bool enviar_mensagem(Mensagem* msg) {
	if (!cliente_state.conectado) return false;

	msg->jogador_id = cliente_state.id;
	msg->sala_id = cliente_state.sala_id;

	int bytes = send(cliente_state.socket, msg, sizeof(Mensagem), 0);
	return bytes == sizeof(Mensagem);
}

void processar_mensagem_recebida(Mensagem* msg) {
	pthread_mutex_lock(&cliente_state.mutex);

	switch (msg->tipo) {
		case MSG_CONECTAR:
			cliente_state.id = msg->jogador_id;
			printf("\nConectado ao servidor! Seu ID: %u\n", cliente_state.id);
			break;

		case MSG_CRIAR_SALA:
			cliente_state.sala_id = msg->sala_id;
			printf("\nSala criada com sucesso! ID: %u\n", cliente_state.sala_id);
			printf("Aguardando outro jogador...\n");
			break;

		case MSG_ENTRAR_SALA:
			if (msg->jogador_id != cliente_state.id) {
				printf("\nOutro jogador entrou na sala!\n");
			} else {
				cliente_state.sala_id = msg->sala_id;
				printf("\nVocê entrou na sala %u\n", cliente_state.sala_id);
			}
			break;

		case MSG_ESTADO_JOGO:
			memcpy(&cliente_state.estado, msg->dados, sizeof(EstadoJogo));
			cliente_state.em_partida = true;
			printf("\n[Estado do jogo atualizado]\n");
			break;

		case MSG_TRUCO:
			printf("\n!!! TRUCO CANTADO !!!\n");
			printf("Responda: 1-Quero 2-Não quero 3-Retruco 4-Vale quatro\n");
			break;

		case MSG_ENVIDO:
			printf("\n!!! ENVIDO CANTADO !!!\n");
			printf("Responda: 1-Quero 2-Não quero 3-Real Envido 4-Falta Envido\n");
			break;

		case MSG_FLOR:
			printf("\n!!! FLOR CANTADA !!!\n");
			printf("Responda: 1-Quero 2-Não quero 3-Contraflor\n");
			break;

		case MSG_FIM_PARTIDA: {
			int vencedor;
			memcpy(&vencedor, msg->dados, sizeof(int));
			printf("\n========================================\n");
			printf("         FIM DE PARTIDA!\n");
			printf("========================================\n");
			if (vencedor == 1) {
				printf("VOCÊ VENCEU!\n");
			} else {
				printf("VOCÊ PERDEU!\n");
			}
			printf("========================================\n");
			cliente_state.em_partida = false;
			break;
		}

		case MSG_ERRO:
			printf("\n[ERRO] Operação falhou.\n");
			break;

		default:
			break;
	}

	pthread_mutex_unlock(&cliente_state.mutex);
}

void* thread_receber_mensagens(void* arg __attribute__((unused))) {
	Mensagem msg;

	while (cliente_state.conectado) {
		int bytes = recv(cliente_state.socket, &msg, sizeof(Mensagem), 0);

		if (bytes == sizeof(Mensagem)) {
			processar_mensagem_recebida(&msg);
		} else if (bytes == 0) {
			printf("\n[Servidor desconectado]\n");
			cliente_state.conectado = false;
			break;
		}
	}

	return NULL;
}

void criar_sala() {
	char nome[64];
	printf("\nNome da sala: ");
	scanf("%s", nome);
	getchar();  // Limpa buffer

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_CRIAR_SALA;
	memcpy(msg.dados, nome, sizeof(nome));

	enviar_mensagem(&msg);

	sleep(1);  // Aguarda resposta
}

void listar_salas() {
	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_LISTAR_SALAS;

	enviar_mensagem(&msg);

	sleep(1);  // Aguarda resposta

	// Por simplicidade, vamos apenas solicitar - a exibição virá pela thread
	printf("\n[Solicitação de listagem enviada]\n");
	printf("As salas disponíveis aparecerão em breve...\n");
	sleep(2);
}

void entrar_sala() {
	uint32_t sala_id;
	printf("\nID da sala: ");
	scanf("%u", &sala_id);
	getchar();

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_ENTRAR_SALA;
	memcpy(msg.dados, &sala_id, sizeof(uint32_t));

	enviar_mensagem(&msg);

	sleep(1);
}

void iniciar_partida() {
	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_INICIAR_PARTIDA;

	enviar_mensagem(&msg);

	printf("\nIniciando partida...\n");
	sleep(2);
}

void jogar_turno() {
	pthread_mutex_lock(&cliente_state.mutex);

	if (cliente_state.estado.vez_jogador != 1) {
		printf("\nNão é sua vez!\n");
		pthread_mutex_unlock(&cliente_state.mutex);
		sleep(2);
		return;
	}

	exibir_estado_jogo();

	printf("\nEscolha uma carta para jogar (1-%d) ou 0 para ir ao baralho: ",
	       cliente_state.estado.num_cartas_mao);

	int escolha;
	scanf("%d", &escolha);
	getchar();

	if (escolha == 0) {
		Mensagem msg;
		memset(&msg, 0, sizeof(Mensagem));
		msg.tipo = MSG_IR_BARALHO;
		enviar_mensagem(&msg);
	} else if (escolha >= 1 && escolha <= cliente_state.estado.num_cartas_mao) {
		int indice = escolha - 1;

		Mensagem msg;
		memset(&msg, 0, sizeof(Mensagem));
		msg.tipo = MSG_JOGAR_CARTA;
		memcpy(msg.dados, &indice, sizeof(int));

		enviar_mensagem(&msg);
	} else {
		printf("Escolha inválida!\n");
		sleep(1);
	}

	pthread_mutex_unlock(&cliente_state.mutex);
	sleep(1);
}

void menu_cantos() {
	pthread_mutex_lock(&cliente_state.mutex);

	limpar_tela();
	printf("========================================\n");
	printf("              CANTOS                    \n");
	printf("========================================\n\n");

	if (cliente_state.estado.aguardando_resposta) {
		printf("Você precisa responder a um canto!\n\n");
		printf("Escolha sua resposta:\n");
		printf("1 - Quero\n");
		printf("2 - Não quero\n");
		printf("3 - Aumentar aposta\n");
		printf("0 - Voltar\n");
		printf("\nOpção: ");

		int resposta;
		scanf("%d", &resposta);
		getchar();

		if (resposta >= 1 && resposta <= 3) {
			Mensagem msg;
			memset(&msg, 0, sizeof(Mensagem));

			// Simplificado - assume resposta ao truco
			msg.tipo = MSG_RESPOSTA_TRUCO;
			RespostaTruco resp = (RespostaTruco)(resposta - 1);
			memcpy(msg.dados, &resp, sizeof(RespostaTruco));

			enviar_mensagem(&msg);
		}
	} else {
		printf("Você pode cantar:\n");

		if (cliente_state.estado.pode_cantar_truco) {
			printf("1 - Truco!\n");
		}
		if (cliente_state.estado.pode_cantar_envido) {
			printf("2 - Envido!\n");
		}
		if (cliente_state.estado.pode_cantar_flor) {
			printf("3 - Flor!\n");
		}

		printf("0 - Voltar\n");
		printf("\nOpção: ");

		int opcao;
		scanf("%d", &opcao);
		getchar();

		Mensagem msg;
		memset(&msg, 0, sizeof(Mensagem));

		switch (opcao) {
			case 1:
				if (cliente_state.estado.pode_cantar_truco) {
					msg.tipo = MSG_TRUCO;
					enviar_mensagem(&msg);
					printf("\nTRUCO cantado!\n");
					sleep(1);
				}
				break;

			case 2:
				if (cliente_state.estado.pode_cantar_envido) {
					msg.tipo = MSG_ENVIDO;
					enviar_mensagem(&msg);
					printf("\nENVIDO cantado!\n");
					sleep(1);
				}
				break;

			case 3:
				if (cliente_state.estado.pode_cantar_flor) {
					msg.tipo = MSG_FLOR;
					enviar_mensagem(&msg);
					printf("\nFLOR cantada!\n");
					sleep(1);
				}
				break;
		}
	}

	pthread_mutex_unlock(&cliente_state.mutex);
}

int main(int argc, char* argv[]) {
	char ip[16] = "127.0.0.1";
	int porta = PORTA_PADRAO;

	if (argc > 1) {
		strncpy(ip, argv[1], sizeof(ip) - 1);
	}
	if (argc > 2) {
		porta = atoi(argv[2]);
	}

	printf("Conectando ao servidor %s:%d...\n", ip, porta);

	if (!conectar_servidor(ip, porta)) {
		printf("Falha ao conectar ao servidor.\n");
		return 1;
	}

	sleep(1);  // Aguarda mensagem de conexão

	// Loop principal
	int opcao;
	while (cliente_state.conectado) {
		exibir_menu_principal();

		scanf("%d", &opcao);
		getchar();  // Limpa buffer

		switch (opcao) {
			case 0:
				desconectar_servidor();
				printf("\nDesconectado. Até logo!\n");
				return 0;

			case 1:
				criar_sala();
				break;

			case 2:
				listar_salas();
				break;

			case 3:
				entrar_sala();
				break;

			case 4:
				if (cliente_state.sala_id > 0 && !cliente_state.em_partida) {
					iniciar_partida();
				}
				break;

			case 5:
				if (cliente_state.em_partida) {
					jogar_turno();
				}
				break;

			case 6:
				if (cliente_state.em_partida) {
					menu_cantos();
				}
				break;

			default:
				printf("\nOpção inválida!\n");
				sleep(1);
				break;
		}
	}

	return 0;
}
