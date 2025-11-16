#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "ui_grafica.h"

// Estrutura do cliente gráfico
typedef struct {
	int socket;
	uint32_t id;
	bool conectado;
	UIGrafica ui;
	UIEstado estado;
	pthread_t thread_recebimento;
	pthread_mutex_t mutex_estado;
} ClienteGrafico;

static ClienteGrafico cliente;

// Protótipos de funções
bool conectar_servidor(const char* ip, int porta);
void desconectar_servidor();
bool enviar_mensagem(Mensagem* msg);
void* thread_receber_mensagens(void* arg);
void processar_mensagem_recebida(Mensagem* msg);

// Callbacks dos botões
void callback_criar_sala(void* data);
void callback_listar_salas(void* data);
void callback_entrar_sala(void* data);
void callback_iniciar_partida(void* data);
void callback_voltar_menu(void* data);
void callback_jogar_carta(void* data);
void callback_truco(void* data);
void callback_envido(void* data);
void callback_flor(void* data);
void callback_quero(void* data);
void callback_nao_quero(void* data);
void callback_retruco(void* data);
void callback_vale_quatro(void* data);
void callback_real_envido(void* data);
void callback_falta_envido(void* data);
void callback_sair(void* data);

// Implementações
bool conectar_servidor(const char* ip, int porta) {
	cliente.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (cliente.socket < 0) {
		perror("Erro ao criar socket");
		return false;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porta);

	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
		perror("Endereço inválido");
		close(cliente.socket);
		return false;
	}

	if (connect(cliente.socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Erro ao conectar");
		close(cliente.socket);
		return false;
	}

	cliente.conectado = true;
	pthread_mutex_init(&cliente.mutex_estado, NULL);

	// Inicia thread de recebimento
	pthread_create(&cliente.thread_recebimento, NULL, thread_receber_mensagens, NULL);

	printf("Conectado ao servidor %s:%d\n", ip, porta);
	return true;
}

void desconectar_servidor() {
	if (cliente.conectado) {
		Mensagem msg;
		memset(&msg, 0, sizeof(Mensagem));
		msg.tipo = MSG_DESCONECTAR;
		enviar_mensagem(&msg);

		cliente.conectado = false;
		close(cliente.socket);
		pthread_cancel(cliente.thread_recebimento);
		pthread_mutex_destroy(&cliente.mutex_estado);
	}
}

bool enviar_mensagem(Mensagem* msg) {
	if (!cliente.conectado) return false;

	msg->jogador_id = cliente.id;
	msg->sala_id = cliente.estado.sala_id;

	int bytes = send(cliente.socket, msg, sizeof(Mensagem), 0);
	return bytes == sizeof(Mensagem);
}

void processar_mensagem_recebida(Mensagem* msg) {
	pthread_mutex_lock(&cliente.mutex_estado);

	switch (msg->tipo) {
		case MSG_CONECTAR:
			cliente.id = msg->jogador_id;
			printf("Conectado! ID: %u\n", cliente.id);
			snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
			         "Conectado! ID: %u", cliente.id);
			cliente.estado.tempo_mensagem = 3.0f;
			break;

		case MSG_CRIAR_SALA:
			cliente.estado.sala_id = msg->sala_id;
			cliente.estado.tela_atual = TELA_LOBBY;
			printf("Sala criada: %u\n", cliente.estado.sala_id);
			snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
			         "Sala criada: %u", cliente.estado.sala_id);
			cliente.estado.tempo_mensagem = 3.0f;
			break;

		case MSG_ENTRAR_SALA:
			if (msg->jogador_id != cliente.id) {
				printf("Outro jogador entrou na sala!\n");
				snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
				         "Jogador entrou! Pronto para iniciar");
				cliente.estado.tempo_mensagem = 3.0f;
			} else {
				cliente.estado.sala_id = msg->sala_id;
				cliente.estado.tela_atual = TELA_LOBBY;
				printf("Entrou na sala %u\n", cliente.estado.sala_id);
			}
			break;

		case MSG_LISTAR_SALAS: {
			int num = msg->tamanho_dados / sizeof(InfoSala);
			cliente.estado.num_salas = num;
			memcpy(cliente.estado.salas, msg->dados, msg->tamanho_dados);
			cliente.estado.tela_atual = TELA_LISTAR_SALAS;
			cliente.estado.precisa_reconfigurar_botoes = true;
			printf("Recebidas %d salas\n", num);
			break;
		}
		case MSG_ESTADO_JOGO:
			memcpy(&cliente.estado.estado_jogo, msg->dados, sizeof(EstadoJogo));

			// Atualiza aguardando_resposta_canto baseado no estado recebido
			cliente.estado.aguardando_resposta_canto = cliente.estado.estado_jogo.aguardando_resposta;

			// Reset carta selecionada se o índice está inválido
			if (cliente.ui.carta_selecionada >= cliente.estado.estado_jogo.num_cartas_mao) {
				printf("Resetando carta_selecionada (era %d, mas só há %d cartas)\n",
				       cliente.ui.carta_selecionada, cliente.estado.estado_jogo.num_cartas_mao);
				cliente.ui.carta_selecionada = -1;
			}

			cliente.estado.em_partida = true;
			cliente.estado.tela_atual = TELA_JOGO;
			cliente.estado.precisa_reconfigurar_botoes = true;
			printf("Estado recebido - Vez: %d, Mao: %d, NumCartas: %d, Pode_Truco: %d\n",
			       cliente.estado.estado_jogo.vez_jogador,
			       cliente.estado.estado_jogo.mao_jogador,
			       cliente.estado.estado_jogo.num_cartas_mao,
			       cliente.estado.estado_jogo.pode_cantar_truco);
			printf("  Mesa R1:[%d-%d][%d-%d] R2:[%d-%d][%d-%d] R3:[%d-%d][%d-%d]\n",
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[0].naipe,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[0].numero,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[1].naipe,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[1].numero,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[2].naipe,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[2].numero,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[3].naipe,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[3].numero,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[4].naipe,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[4].numero,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[5].naipe,
			       cliente.estado.estado_jogo.cartas_jogadas_rodada[5].numero);
			break;
		case MSG_TRUCO:
			printf("TRUCO cantado!\n");
			cliente.estado.tipo_canto_aguardando = MSG_TRUCO;
			snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
			         "TRUCO! Responda!");
			cliente.estado.tempo_mensagem = 5.0f;
			break;
		case MSG_ENVIDO:
			printf("ENVIDO cantado!\n");
			cliente.estado.tipo_canto_aguardando = MSG_ENVIDO;
			snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
			         "ENVIDO! Responda!");
			cliente.estado.tempo_mensagem = 5.0f;
			break;
		case MSG_FLOR:
			printf("FLOR cantada!\n");
			cliente.estado.tipo_canto_aguardando = MSG_FLOR;
			snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
			         "FLOR! Responda!");
			cliente.estado.tempo_mensagem = 5.0f;
			break;
		case MSG_FIM_PARTIDA: {
			int vencedor;
			memcpy(&vencedor, msg->dados, sizeof(int));
			cliente.estado.vencedor_partida = vencedor;
			cliente.estado.tela_atual = TELA_FIM_PARTIDA;
			cliente.estado.em_partida = false;
			printf("Partida finalizada! Vencedor: %d\n", vencedor);
			break;
		}

		case MSG_ERRO:
			printf("Erro recebido do servidor\n");
			snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
			         "Erro na operacao!");
			cliente.estado.tempo_mensagem = 3.0f;
			break;

		default:
			break;
	}

	pthread_mutex_unlock(&cliente.mutex_estado);
}

void* thread_receber_mensagens(void* arg) {
	(void)arg;
	Mensagem msg;

	while (cliente.conectado) {
		int bytes = recv(cliente.socket, &msg, sizeof(Mensagem), 0);

		if (bytes == sizeof(Mensagem)) {
			processar_mensagem_recebida(&msg);
		} else if (bytes == 0) {
			printf("Servidor desconectado\n");
			cliente.conectado = false;
			break;
		}
	}

	return NULL;
}

// Implementação dos callbacks
void callback_criar_sala(void* data) {
	(void)data;
	cliente.estado.tela_atual = TELA_CRIAR_SALA;
	memset(cliente.estado.input_texto, 0, sizeof(cliente.estado.input_texto));
}

void callback_confirmar_criar_sala(void* data) {
	(void)data;

	if (strlen(cliente.estado.input_texto) == 0) {
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Digite um nome para a sala!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_CRIAR_SALA;
	strncpy((char*)msg.dados, cliente.estado.input_texto, sizeof(msg.dados) - 1);

	enviar_mensagem(&msg);
}

void callback_listar_salas(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_LISTAR_SALAS;

	enviar_mensagem(&msg);
	// A tela será mudada quando recebermos MSG_LISTAR_SALAS
}

void callback_entrar_sala(void* data) {
	uint32_t* sala_id = (uint32_t*)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_ENTRAR_SALA;
	memcpy(msg.dados, sala_id, sizeof(uint32_t));

	enviar_mensagem(&msg);
}

void callback_iniciar_partida(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_INICIAR_PARTIDA;

	enviar_mensagem(&msg);
}

void callback_voltar_menu(void* data) {
	(void)data;
	cliente.estado.tela_atual = TELA_MENU_PRINCIPAL;
}

void callback_jogar_carta(void* data) {
	(void)data;

	printf("callback_jogar_carta chamado - carta_selecionada: %d, num_cartas: %d\n",
	       cliente.ui.carta_selecionada, cliente.estado.estado_jogo.num_cartas_mao);

	if (cliente.ui.carta_selecionada < 0) {
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Selecione uma carta!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	// Validação adicional: índice deve ser menor que número de cartas
	if (cliente.ui.carta_selecionada >= cliente.estado.estado_jogo.num_cartas_mao) {
		printf("ERRO: Índice %d inválido para %d cartas!\n",
		       cliente.ui.carta_selecionada, cliente.estado.estado_jogo.num_cartas_mao);
		cliente.ui.carta_selecionada = -1;
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Carta invalida!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	int indice = cliente.ui.carta_selecionada;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_JOGAR_CARTA;
	memcpy(msg.dados, &indice, sizeof(int));

	enviar_mensagem(&msg);

	cliente.ui.carta_selecionada = -1;
}

void callback_truco(void* data) {
	(void)data;

	printf("callback_truco chamado\n");

	// Validações antes de enviar
	if (cliente.estado.estado_jogo.aguardando_resposta) {
		printf("  ERRO: Aguardando resposta\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Aguarde resposta!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	if (!cliente.estado.estado_jogo.pode_cantar_truco) {
		printf("  ERRO: Nao pode cantar TRUCO agora (pode_cantar_truco=0)\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Nao pode cantar TRUCO agora!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	printf("  OK: Enviando MSG_TRUCO ao servidor\n");
	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_TRUCO;

	enviar_mensagem(&msg);
}

void callback_envido(void* data) {
	(void)data;

	printf("callback_envido chamado\n");

	// Validações
	if (cliente.estado.estado_jogo.aguardando_resposta) {
		printf("  ERRO: Aguardando resposta\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Aguarde resposta!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	if (cliente.estado.estado_jogo.rodada_atual > 0) {
		printf("  ERRO: Envido so pode ser cantado na primeira rodada\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Envido so na primeira rodada!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	if (!cliente.estado.estado_jogo.pode_cantar_envido) {
		printf("  ERRO: Nao pode cantar ENVIDO agora (pode_cantar_envido=0)\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Nao pode cantar ENVIDO agora!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	printf("  OK: Enviando MSG_ENVIDO ao servidor\n");
	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_ENVIDO;

	enviar_mensagem(&msg);
}

void callback_flor(void* data) {
	(void)data;

	printf("callback_flor chamado\n");

	// Validações
	if (cliente.estado.estado_jogo.aguardando_resposta) {
		printf("  ERRO: Aguardando resposta\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Aguarde resposta!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	if (cliente.estado.estado_jogo.rodada_atual > 0) {
		printf("  ERRO: Flor so pode ser cantada na primeira rodada\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Flor so na primeira rodada!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	if (!cliente.estado.estado_jogo.pode_cantar_flor) {
		printf("  ERRO: Nao pode cantar FLOR agora (pode_cantar_flor=0 ou sem flor)\n");
		snprintf(cliente.estado.mensagem_temporaria, sizeof(cliente.estado.mensagem_temporaria),
		         "Nao pode cantar FLOR!");
		cliente.estado.tempo_mensagem = 2.0f;
		return;
	}

	printf("  OK: Enviando MSG_FLOR ao servidor\n");
	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_FLOR;

	enviar_mensagem(&msg);
}

void callback_quero(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));

	if (cliente.estado.tipo_canto_aguardando == MSG_TRUCO) {
		msg.tipo = MSG_RESPOSTA_TRUCO;
		RespostaTruco resp = RESPOSTA_QUERO;
		memcpy(msg.dados, &resp, sizeof(RespostaTruco));
	} else if (cliente.estado.tipo_canto_aguardando == MSG_ENVIDO) {
		msg.tipo = MSG_RESPOSTA_ENVIDO;
		RespostaEnvido resp = ENVIDO_QUERO;
		memcpy(msg.dados, &resp, sizeof(RespostaEnvido));
	} else if (cliente.estado.tipo_canto_aguardando == MSG_FLOR) {
		msg.tipo = MSG_RESPOSTA_FLOR;
		RespostaFlor resp = FLOR_QUERO;
		memcpy(msg.dados, &resp, sizeof(RespostaFlor));
	}

	enviar_mensagem(&msg);
	cliente.estado.aguardando_resposta_canto = false;
}

void callback_nao_quero(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));

	if (cliente.estado.tipo_canto_aguardando == MSG_TRUCO) {
		msg.tipo = MSG_RESPOSTA_TRUCO;
		RespostaTruco resp = RESPOSTA_NAO_QUERO;
		memcpy(msg.dados, &resp, sizeof(RespostaTruco));
	} else if (cliente.estado.tipo_canto_aguardando == MSG_ENVIDO) {
		msg.tipo = MSG_RESPOSTA_ENVIDO;
		RespostaEnvido resp = ENVIDO_NAO_QUERO;
		memcpy(msg.dados, &resp, sizeof(RespostaEnvido));
	} else if (cliente.estado.tipo_canto_aguardando == MSG_FLOR) {
		msg.tipo = MSG_RESPOSTA_FLOR;
		RespostaFlor resp = FLOR_NAO_QUERO;
		memcpy(msg.dados, &resp, sizeof(RespostaFlor));
	}

	enviar_mensagem(&msg);
	cliente.estado.aguardando_resposta_canto = false;
}

void callback_retruco(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_RESPOSTA_TRUCO;
	RespostaTruco resp = RESPOSTA_RETRUCO;
	memcpy(msg.dados, &resp, sizeof(RespostaTruco));

	enviar_mensagem(&msg);
	cliente.estado.aguardando_resposta_canto = false;
}

void callback_vale_quatro(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_RESPOSTA_TRUCO;
	RespostaTruco resp = RESPOSTA_VALE_QUATRO;
	memcpy(msg.dados, &resp, sizeof(RespostaTruco));

	enviar_mensagem(&msg);
	cliente.estado.aguardando_resposta_canto = false;
}

void callback_real_envido(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_RESPOSTA_ENVIDO;
	RespostaEnvido resp = ENVIDO_REAL_ENVIDO;
	memcpy(msg.dados, &resp, sizeof(RespostaEnvido));

	enviar_mensagem(&msg);
	cliente.estado.aguardando_resposta_canto = false;
}

void callback_falta_envido(void* data) {
	(void)data;

	Mensagem msg;
	memset(&msg, 0, sizeof(Mensagem));
	msg.tipo = MSG_RESPOSTA_ENVIDO;
	RespostaEnvido resp = ENVIDO_FALTA_ENVIDO;
	memcpy(msg.dados, &resp, sizeof(RespostaEnvido));

	enviar_mensagem(&msg);
	cliente.estado.aguardando_resposta_canto = false;
}

void callback_sair(void* data) {
	(void)data;
	desconectar_servidor();
	exit(0);
}

void configurar_botoes_menu_principal() {
	ui_limpar_botoes(&cliente.ui);

	int centro_x = LARGURA_JANELA / 2 - 100;
	int y = 300;

	ui_adicionar_botao(&cliente.ui, centro_x, y, 200, 50, "Criar Sala", callback_criar_sala, NULL);
	ui_adicionar_botao(&cliente.ui, centro_x, y + 70, 200, 50, "Listar Salas", callback_listar_salas, NULL);
	ui_adicionar_botao(&cliente.ui, centro_x, y + 140, 200, 50, "Sair", callback_sair, NULL);
}

void configurar_botoes_criar_sala() {
	ui_limpar_botoes(&cliente.ui);

	ui_adicionar_botao(&cliente.ui, 100, 400, 150, 50, "Criar", callback_confirmar_criar_sala, NULL);
	ui_adicionar_botao(&cliente.ui, 270, 400, 150, 50, "Voltar", callback_voltar_menu, NULL);
}

void configurar_botoes_listar_salas() {
	ui_limpar_botoes(&cliente.ui);

	// Adiciona botão para cada sala
	for (int i = 0; i < cliente.estado.num_salas && i < 10; i++) {
		uint32_t* sala_id = malloc(sizeof(uint32_t));
		*sala_id = cliente.estado.salas[i].id;
		ui_adicionar_botao(&cliente.ui, 600, 150 + i * 40, 100, 30,
		                   "Entrar", callback_entrar_sala, sala_id);
	}

	ui_adicionar_botao(&cliente.ui, 100, 600, 150, 50, "Voltar", callback_voltar_menu, NULL);
}

void configurar_botoes_lobby() {
	ui_limpar_botoes(&cliente.ui);

	ui_adicionar_botao(&cliente.ui, 100, 400, 200, 50, "Iniciar Partida", callback_iniciar_partida, NULL);
	ui_adicionar_botao(&cliente.ui, 100, 470, 200, 50, "Voltar", callback_voltar_menu, NULL);
}

void configurar_botoes_jogo() {
	ui_limpar_botoes(&cliente.ui);
	cliente.ui.carta_selecionada = -1;  // Reset seleção

	printf("Configurando botões - Aguardando: %d, Vez: %d, Pode_Truco: %d\n",
	       cliente.estado.aguardando_resposta_canto,
	       cliente.estado.estado_jogo.vez_jogador,
	       cliente.estado.estado_jogo.pode_cantar_truco);

	if (cliente.estado.aguardando_resposta_canto) {
		// Botões de resposta
		int y = 200;
		ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Quero", callback_quero, NULL);
		y += 70;
		ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Nao Quero", callback_nao_quero, NULL);

		// Botões para aumentar aposta
		if (cliente.estado.tipo_canto_aguardando == MSG_TRUCO) {
			y += 70;
			// Mostrar "Retruco!" se valor_rodada==2 (após Truco), "Vale Quatro!" se valor_rodada==3 (após Retruco)
			if (cliente.estado.estado_jogo.valor_rodada == 2) {
				ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Retruco!", callback_retruco, NULL);
			} else if (cliente.estado.estado_jogo.valor_rodada == 3) {
				ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Vale Quatro!", callback_vale_quatro, NULL);
			}
		} else if (cliente.estado.tipo_canto_aguardando == MSG_ENVIDO) {
			y += 70;
			ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Real Envido", callback_real_envido, NULL);
			y += 70;
			ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Falta Envido", callback_falta_envido, NULL);
		}
	} else {
		// Botões de ação
		int y = 200;
		ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Jogar Carta", callback_jogar_carta, NULL);

		if (cliente.estado.estado_jogo.pode_cantar_truco) {
			y += 70;
			ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "TRUCO!", callback_truco, NULL);
		}

		if (cliente.estado.estado_jogo.pode_cantar_envido) {
			y += 70;
			ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Envido", callback_envido, NULL);
		}

		if (cliente.estado.estado_jogo.pode_cantar_flor) {
			y += 70;
			ui_adicionar_botao(&cliente.ui, LARGURA_JANELA - 220, y, 200, 50, "Flor", callback_flor, NULL);
		}
	}
}

void configurar_botoes_fim_partida() {
	ui_limpar_botoes(&cliente.ui);

	ui_adicionar_botao(&cliente.ui, LARGURA_JANELA / 2 - 100, 400, 200, 50,
	                   "Voltar ao Menu", callback_voltar_menu, NULL);
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

	// Inicializar cliente
	memset(&cliente, 0, sizeof(ClienteGrafico));
	cliente.estado.tela_atual = TELA_MENU_PRINCIPAL;

	// Inicializar UI
	if (!ui_inicializar(&cliente.ui)) {
		fprintf(stderr, "Erro ao inicializar interface gráfica\n");
		return 1;
	}

	// Conectar ao servidor
	if (!conectar_servidor(ip, porta)) {
		fprintf(stderr, "Erro ao conectar ao servidor\n");
		ui_finalizar(&cliente.ui);
		return 1;
	}

	sleep(1);  // Aguarda mensagem de conexão

	// Configurar botões iniciais
	configurar_botoes_menu_principal();

	// Loop principal
	bool rodando = true;
	TipoTela tela_anterior = TELA_MENU_PRINCIPAL;

	while (rodando && cliente.conectado) {
		// Processar eventos
		SDL_Event evento;
		while (SDL_PollEvent(&evento)) {
			if (evento.type == SDL_QUIT) {
				rodando = false;
			}

			pthread_mutex_lock(&cliente.mutex_estado);
			ui_processar_evento(&cliente.ui, &cliente.estado, &evento);
			pthread_mutex_unlock(&cliente.mutex_estado);
		}

		// Atualizar botões se mudou de tela OU se foi solicitada reconfiguração
		pthread_mutex_lock(&cliente.mutex_estado);
		if (cliente.estado.tela_atual != tela_anterior || cliente.estado.precisa_reconfigurar_botoes) {
			switch (cliente.estado.tela_atual) {
				case TELA_MENU_PRINCIPAL:
					configurar_botoes_menu_principal();
					break;
				case TELA_CRIAR_SALA:
					configurar_botoes_criar_sala();
					break;
				case TELA_LISTAR_SALAS:
					configurar_botoes_listar_salas();
					break;
				case TELA_LOBBY:
					configurar_botoes_lobby();
					break;
				case TELA_JOGO:
					configurar_botoes_jogo();
					break;
				case TELA_FIM_PARTIDA:
					configurar_botoes_fim_partida();
					break;
			}
			tela_anterior = cliente.estado.tela_atual;
			cliente.estado.precisa_reconfigurar_botoes = false;
		}

		// Renderizar
		ui_renderizar(&cliente.ui, &cliente.estado);
		pthread_mutex_unlock(&cliente.mutex_estado);

		SDL_Delay(16);  // ~60 FPS
	}

	// Limpar
	desconectar_servidor();
	ui_finalizar(&cliente.ui);

	return 0;
}
