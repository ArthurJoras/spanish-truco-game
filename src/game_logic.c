#include "game_logic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Inicializar baralho com 40 cartas (sem 8 e 9)
void inicializar_baralho(Baralho* baralho) {
	int index = 0;
	for (int naipe = NAIPE_ESPADAS; naipe <= NAIPE_OUROS; naipe++) {
		NumeroCarta numeros[] = {NUMERO_AS, NUMERO_2, NUMERO_3, NUMERO_4, NUMERO_5,
		                         NUMERO_6, NUMERO_7, NUMERO_10, NUMERO_11, NUMERO_12};
		for (int i = 0; i < 10; i++) {
			baralho->cartas[index].naipe = (Naipe)naipe;
			baralho->cartas[index].numero = numeros[i];
			index++;
		}
	}
	baralho->topo = 0;
}

void embaralhar(Baralho* baralho) {
	srand(time(NULL));
	for (int i = 39; i > 0; i--) {
		int j = rand() % (i + 1);
		Carta temp = baralho->cartas[i];
		baralho->cartas[i] = baralho->cartas[j];
		baralho->cartas[j] = temp;
	}
	baralho->topo = 0;
}

Carta pegar_carta(Baralho* baralho) {
	return baralho->cartas[baralho->topo++];
}

void inicializar_jogo(Jogo* jogo, uint32_t sala_id) {
	memset(jogo, 0, sizeof(Jogo));
	jogo->sala_id = sala_id;
	jogo->pontos_jogador1 = 0;
	jogo->pontos_jogador2 = 0;
	jogo->valor_rodada = 1;
	jogo->mao_jogador = 1;
	jogo->vez_jogador = 1;  // Jogador 1 começa
	jogo->partida_finalizada = false;
}

void distribuir_cartas(Jogo* jogo) {
	embaralhar(&jogo->baralho);

	// Distribui 3 cartas para cada jogador
	for (int i = 0; i < 3; i++) {
		jogo->jogador1.mao[i] = pegar_carta(&jogo->baralho);
		jogo->jogador2.mao[i] = pegar_carta(&jogo->baralho);
	}
	jogo->jogador1.num_cartas = 3;
	jogo->jogador2.num_cartas = 3;

	// Calcula envido e flor
	jogo->jogador1.pontos_envido = calcular_pontos_envido(&jogo->jogador1);
	jogo->jogador2.pontos_envido = calcular_pontos_envido(&jogo->jogador2);
	jogo->jogador1.tem_flor = verificar_flor(&jogo->jogador1);
	jogo->jogador2.tem_flor = verificar_flor(&jogo->jogador2);
}

void nova_mao(Jogo* jogo) {
	// Reset para nova mão
	memset(jogo->rodadas, 0, sizeof(jogo->rodadas));
	jogo->rodada_atual = 0;
	jogo->valor_rodada = 1;
	jogo->truco_cantado = false;
	jogo->envido_cantado = false;
	jogo->flor_cantada = false;
	jogo->aguardando_resposta_truco = false;
	jogo->aguardando_resposta_envido = false;
	jogo->aguardando_resposta_flor = false;
	jogo->jogador_cantou_truco = 0;
	jogo->jogador_cantou_envido = 0;
	jogo->jogador_cantou_flor = 0;
	jogo->ultimo_a_aumentar_truco = 0;
	jogo->ultimo_a_aumentar_envido = 0;

	// Quem era mão vira pé
	jogo->mao_jogador = (jogo->mao_jogador == 1) ? 2 : 1;
	jogo->vez_jogador = jogo->mao_jogador;

	// Distribui novas cartas
	inicializar_baralho(&jogo->baralho);
	distribuir_cartas(jogo);
}

// Hierarquia do truco espanhol (do mais forte ao mais fraco)
int obter_valor_carta_truco(Carta carta) {
	// As de espadas (Espada Ancha) - mais forte
	if (carta.numero == NUMERO_AS && carta.naipe == NAIPE_ESPADAS) return 14;
	// As de paus (Espadão)
	if (carta.numero == NUMERO_AS && carta.naipe == NAIPE_PAUS) return 13;
	// 7 de espadas
	if (carta.numero == NUMERO_7 && carta.naipe == NAIPE_ESPADAS) return 12;
	// 7 de ouros
	if (carta.numero == NUMERO_7 && carta.naipe == NAIPE_OUROS) return 11;
	// Todos os 3s
	if (carta.numero == NUMERO_3) return 10;
	// Todos os 2s
	if (carta.numero == NUMERO_2) return 9;
	// As de copas e ouros
	if (carta.numero == NUMERO_AS) return 8;
	// Reis (12)
	if (carta.numero == NUMERO_12) return 7;
	// Cavalos (11)
	if (carta.numero == NUMERO_11) return 6;
	// Valetes (10)
	if (carta.numero == NUMERO_10) return 5;
	// 7 de paus e copas
	if (carta.numero == NUMERO_7) return 4;
	// 6
	if (carta.numero == NUMERO_6) return 3;
	// 5
	if (carta.numero == NUMERO_5) return 2;
	// 4
	if (carta.numero == NUMERO_4) return 1;

	return 0;
}

int comparar_cartas_truco(Carta c1, Carta c2) {
	int v1 = obter_valor_carta_truco(c1);
	int v2 = obter_valor_carta_truco(c2);

	if (v1 > v2) return 1;
	if (v1 < v2) return -1;
	return 0;  // Empate
}

int calcular_pontos_envido(Jogador* jogador) {
	int pontos_por_naipe[4] = {0, 0, 0, 0};
	int cartas_por_naipe[4] = {0, 0, 0, 0};

	// Conta cartas e pontos por naipe
	for (int i = 0; i < jogador->num_cartas; i++) {
		Carta c = jogador->mao[i];
		int valor = (c.numero <= 7) ? c.numero : 0;  // Figuras valem 0
		pontos_por_naipe[c.naipe] += valor;
		cartas_por_naipe[c.naipe]++;
	}

	// Procura o maior envido
	int maior_envido = 0;
	for (int i = 0; i < 4; i++) {
		if (cartas_por_naipe[i] >= 2) {
			int envido = pontos_por_naipe[i] + 20;
			if (envido > maior_envido) maior_envido = envido;
		}
	}

	// Se não tem duas cartas do mesmo naipe, pega a maior carta
	if (maior_envido == 0) {
		for (int i = 0; i < jogador->num_cartas; i++) {
			int valor = (jogador->mao[i].numero <= 7) ? jogador->mao[i].numero : 0;
			if (valor > maior_envido) maior_envido = valor;
		}
	}

	return maior_envido;
}

bool verificar_flor(Jogador* jogador) {
	if (jogador->num_cartas < 3) return false;

	Naipe primeiro_naipe = jogador->mao[0].naipe;
	for (int i = 1; i < 3; i++) {
		if (jogador->mao[i].naipe != primeiro_naipe) return false;
	}
	return true;
}

bool pode_jogar_carta(Jogo* jogo, int jogador, int indice_carta) {
	if (jogo->partida_finalizada) return false;
	if (jogo->vez_jogador != jogador) return false;
	if (jogo->aguardando_resposta_truco || jogo->aguardando_resposta_envido ||
	    jogo->aguardando_resposta_flor) return false;

	Jogador* j = (jogador == 1) ? &jogo->jogador1 : &jogo->jogador2;
	if (indice_carta < 0 || indice_carta >= j->num_cartas) return false;

	return true;
}

bool jogar_carta(Jogo* jogo, int jogador, int indice_carta) {
	if (!pode_jogar_carta(jogo, jogador, indice_carta)) return false;

	Jogador* j = (jogador == 1) ? &jogo->jogador1 : &jogo->jogador2;
	Rodada* rodada = &jogo->rodadas[jogo->rodada_atual];

	Carta carta_jogada = j->mao[indice_carta];

	// Remove carta da mão
	for (int i = indice_carta; i < j->num_cartas - 1; i++) {
		j->mao[i] = j->mao[i + 1];
	}
	j->num_cartas--;

	// Registra a jogada
	if (jogador == 1) {
		rodada->carta_jogador1 = carta_jogada;
		rodada->jogador1_jogou = true;
	} else {
		rodada->carta_jogador2 = carta_jogada;
		rodada->jogador2_jogou = true;
	}

	// Se ambos jogaram, resolve a rodada
	if (rodada->jogador1_jogou && rodada->jogador2_jogou) {
		resolver_rodada(jogo);
	} else {
		// Passa a vez
		jogo->vez_jogador = (jogador == 1) ? 2 : 1;
	}

	return true;
}

void resolver_rodada(Jogo* jogo) {
	Rodada* rodada = &jogo->rodadas[jogo->rodada_atual];

	int resultado = comparar_cartas_truco(rodada->carta_jogador1, rodada->carta_jogador2);

	if (resultado > 0) {
		rodada->vencedor = 1;
		jogo->vez_jogador = 1;
	} else if (resultado < 0) {
		rodada->vencedor = 2;
		jogo->vez_jogador = 2;
	} else {
		rodada->vencedor = 0;  // Empate
		// Em caso de empate, quem é mão joga primeiro na próxima
		jogo->vez_jogador = jogo->mao_jogador;
	}

	jogo->rodada_atual++;

	// Verifica se a mão terminou
	if (jogo->rodada_atual >= 3) {
		finalizar_mao(jogo);
	} else {
		// Verifica se já tem vencedor antes de 3 rodadas
		// Regras do truco espanhol:
		// - 2 vitórias = ganha
		// - Empate na 1ª + vitória na 2ª = quem ganhou a 2ª ganha a mão
		// - Vitória na 1ª + empate na 2ª = quem ganhou a 1ª ganha a mão
		int vitorias_j1 = 0, vitorias_j2 = 0, empates = 0;
		for (int i = 0; i < jogo->rodada_atual; i++) {
			if (jogo->rodadas[i].vencedor == 1)
				vitorias_j1++;
			else if (jogo->rodadas[i].vencedor == 2)
				vitorias_j2++;
			else
				empates++;
		}

		// Condições de finalização antes de 3 rodadas:
		// 1) Alguém tem 2 vitórias
		// 2) Após 2 rodadas: 1 vitória + 1 empate (quem venceu ganha a mão)
		bool finalizar = false;
		if (vitorias_j1 >= 2 || vitorias_j2 >= 2) {
			finalizar = true;
		} else if (jogo->rodada_atual == 2 && empates == 1 && (vitorias_j1 == 1 || vitorias_j2 == 1)) {
			// Uma vitória e um empate após 2 rodadas = mão decidida
			finalizar = true;
		}

		if (finalizar) {
			finalizar_mao(jogo);
		}
	}
}

void finalizar_mao(Jogo* jogo) {
	// Conta vitórias
	int vitorias_j1 = 0, vitorias_j2 = 0;
	for (int i = 0; i < jogo->rodada_atual; i++) {
		if (jogo->rodadas[i].vencedor == 1)
			vitorias_j1++;
		else if (jogo->rodadas[i].vencedor == 2)
			vitorias_j2++;
	}

	// Determina vencedor da mão
	int vencedor;
	if (vitorias_j1 > vitorias_j2) {
		vencedor = 1;
	} else if (vitorias_j2 > vitorias_j1) {
		vencedor = 2;
	} else {
		// Empate - quem é mão ganha
		vencedor = jogo->mao_jogador;
	}

	// Adiciona pontos
	if (vencedor == 1) {
		jogo->pontos_jogador1 += jogo->valor_rodada;
	} else {
		jogo->pontos_jogador2 += jogo->valor_rodada;
	}

	// Verifica fim de partida
	if (!verificar_fim_partida(jogo)) {
		nova_mao(jogo);
	}
}

bool pode_cantar_truco(Jogo* jogo, int jogador) {
	if (jogo->partida_finalizada) return false;
	if (jogo->truco_cantado) return false;
	if (jogo->aguardando_resposta_truco || jogo->aguardando_resposta_envido ||
	    jogo->aguardando_resposta_flor) return false;
	if (jogo->rodada_atual >= 3) return false;

	// Só pode cantar truco se for sua vez ou se o oponente jogou primeiro
	Rodada* rodada = &jogo->rodadas[jogo->rodada_atual];
	if (jogador == 1 && rodada->jogador1_jogou && !rodada->jogador2_jogou) return false;
	if (jogador == 2 && rodada->jogador2_jogou && !rodada->jogador1_jogou) return false;

	return true;
}

bool cantar_truco(Jogo* jogo, int jogador) {
	if (!pode_cantar_truco(jogo, jogador)) return false;

	jogo->truco_cantado = true;
	jogo->aguardando_resposta_truco = true;
	jogo->jogador_cantou_truco = jogador;
	jogo->ultimo_a_aumentar_truco = jogador;

	return true;
}

void responder_truco(Jogo* jogo, int jogador, RespostaTruco resposta) {
	if (!jogo->aguardando_resposta_truco) return;
	if (jogador == jogo->jogador_cantou_truco) return;  // Não pode responder próprio truco

	jogo->aguardando_resposta_truco = false;

	switch (resposta) {
		case RESPOSTA_QUERO:
			jogo->valor_rodada = (jogo->valor_rodada == 1) ? 2 : (jogo->valor_rodada == 2) ? 3
			                                                                               : 4;
			break;

		case RESPOSTA_NAO_QUERO:
			// Quem cantou truco ganha os pontos anteriores (valor_rodada - 1)
			// Truco (valor=2) dá 1, Retruco (valor=3) dá 2, Vale4 (valor=4) dá 3
			int pontos_truco = jogo->valor_rodada - 1;
			if (jogo->jogador_cantou_truco == 1) {
				jogo->pontos_jogador1 += pontos_truco;
			} else {
				jogo->pontos_jogador2 += pontos_truco;
			}
			verificar_fim_partida(jogo);
			if (!jogo->partida_finalizada) nova_mao(jogo);
			break;

		case RESPOSTA_RETRUCO:
			// Retruco incrementa para 3
			jogo->valor_rodada = 3;
			jogo->aguardando_resposta_truco = true;
			jogo->jogador_cantou_truco = jogador;
			jogo->ultimo_a_aumentar_truco = jogador;
			break;

		case RESPOSTA_VALE_QUATRO:
			// Vale Quatro incrementa para 4
			jogo->valor_rodada = 4;
			jogo->aguardando_resposta_truco = true;
			jogo->jogador_cantou_truco = jogador;
			jogo->ultimo_a_aumentar_truco = jogador;
			break;
	}
}

bool pode_cantar_envido(Jogo* jogo, int jogador) {
	if (jogo->partida_finalizada) return false;
	if (jogo->envido_cantado) return false;
	if (jogo->rodada_atual > 0) return false;  // Só na primeira rodada
	if (jogo->aguardando_resposta_truco || jogo->aguardando_resposta_envido || jogo->aguardando_resposta_flor) return false;

	Rodada* rodada = &jogo->rodadas[0];
	if (rodada->jogador1_jogou && rodada->jogador2_jogou) return false;

	// Só pode cantar envido se for sua vez ou se o oponente jogou primeiro
	if (jogador == 1 && rodada->jogador1_jogou && !rodada->jogador2_jogou) return false;
	if (jogador == 2 && rodada->jogador2_jogou && !rodada->jogador1_jogou) return false;

	return true;
}

bool cantar_envido(Jogo* jogo, int jogador) {
	if (!pode_cantar_envido(jogo, jogador)) return false;

	jogo->envido_cantado = true;
	jogo->aguardando_resposta_envido = true;
	jogo->jogador_cantou_envido = jogador;
	jogo->ultimo_a_aumentar_envido = jogador;
	jogo->valor_envido = 2;  // Envido base vale 2

	return true;
}

void responder_envido(Jogo* jogo, int jogador, RespostaEnvido resposta) {
	if (!jogo->aguardando_resposta_envido) return;
	if (jogador == jogo->jogador_cantou_envido) return;

	jogo->aguardando_resposta_envido = false;

	switch (resposta) {
		case ENVIDO_QUERO: {
			// Compara pontos de envido
			int pontos_j1 = jogo->jogador1.pontos_envido;
			int pontos_j2 = jogo->jogador2.pontos_envido;

			// Usa valor_envido: 2=Envido, 3=Real Envido, 4=Falta Envido
			int pontos_ganhos = jogo->valor_envido;

			// Falta Envido: vale os pontos que faltam para 15 MENOS 1
			if (jogo->valor_envido == 4) {
				int pontos_oponente = (jogo->jogador_cantou_envido == 1) ? jogo->pontos_jogador2 : jogo->pontos_jogador1;
				pontos_ganhos = 15 - pontos_oponente - 1;
				if (pontos_ganhos < 1) pontos_ganhos = 1;  // Mínimo 1 ponto
			}
			if (pontos_j1 > pontos_j2) {
				jogo->pontos_jogador1 += pontos_ganhos;
			} else if (pontos_j2 > pontos_j1) {
				jogo->pontos_jogador2 += pontos_ganhos;
			} else {
				// Empate - quem é mão ganha
				if (jogo->mao_jogador == 1) {
					jogo->pontos_jogador1 += pontos_ganhos;
				} else {
					jogo->pontos_jogador2 += pontos_ganhos;
				}
			}
			verificar_fim_partida(jogo);
			break;
		}

		case ENVIDO_NAO_QUERO:
			// Quem cantou ganha os pontos acumulados (valor_envido - 1)
			// Envido=2 dá 1, Real=3 dá 2, Falta=4 dá 3
			int pontos = jogo->valor_envido - 1;
			if (jogo->jogador_cantou_envido == 1) {
				jogo->pontos_jogador1 += pontos;
			} else {
				jogo->pontos_jogador2 += pontos;
			}
			verificar_fim_partida(jogo);
			break;
		case ENVIDO_REAL_ENVIDO:
			// Real Envido aumenta para 3
			jogo->valor_envido = 3;
			jogo->aguardando_resposta_envido = true;
			jogo->jogador_cantou_envido = jogador;
			jogo->ultimo_a_aumentar_envido = jogador;
			break;

		case ENVIDO_FALTA_ENVIDO:
			// Falta Envido aumenta para 4
			jogo->valor_envido = 4;
			jogo->aguardando_resposta_envido = true;
			jogo->jogador_cantou_envido = jogador;
			jogo->ultimo_a_aumentar_envido = jogador;
			break;
	}
}

bool pode_cantar_flor(Jogo* jogo, int jogador) {
	if (jogo->partida_finalizada) return false;
	if (jogo->flor_cantada) return false;
	if (jogo->rodada_atual > 0) return false;
	if (jogo->aguardando_resposta_truco || jogo->aguardando_resposta_envido || jogo->aguardando_resposta_flor) return false;

	Rodada* rodada = &jogo->rodadas[0];
	if (rodada->jogador1_jogou && rodada->jogador2_jogou) return false;

	// Só pode cantar flor se for sua vez ou se o oponente jogou primeiro
	if (jogador == 1 && rodada->jogador1_jogou && !rodada->jogador2_jogou) return false;
	if (jogador == 2 && rodada->jogador2_jogou && !rodada->jogador1_jogou) return false;

	Jogador* j = (jogador == 1) ? &jogo->jogador1 : &jogo->jogador2;
	return j->tem_flor;
}

bool cantar_flor(Jogo* jogo, int jogador) {
	if (!pode_cantar_flor(jogo, jogador)) return false;

	// No truco espanhol, se um jogador tem flor, ele automaticamente ganha 3 pontos
	// Não há "resposta" à flor como no envido
	jogo->flor_cantada = true;

	// Dá 3 pontos direto para quem cantou
	if (jogador == 1) {
		jogo->pontos_jogador1 += 3;
	} else {
		jogo->pontos_jogador2 += 3;
	}

	verificar_fim_partida(jogo);
	return true;
}

void responder_flor(Jogo* jogo, int jogador, RespostaFlor resposta) {
	if (!jogo->aguardando_resposta_flor) return;
	if (jogador == jogo->jogador_cantou_flor) return;

	jogo->aguardando_resposta_flor = false;

	// Implementação simplificada da flor
	switch (resposta) {
		case FLOR_QUERO:
			if (jogo->jogador_cantou_flor == 1) {
				jogo->pontos_jogador1 += 3;
			} else {
				jogo->pontos_jogador2 += 3;
			}
			verificar_fim_partida(jogo);
			break;

		case FLOR_NAO_QUERO:
			if (jogo->jogador_cantou_flor == 1) {
				jogo->pontos_jogador1 += 3;
			} else {
				jogo->pontos_jogador2 += 3;
			}
			verificar_fim_partida(jogo);
			break;

		default:
			break;
	}
}

bool pode_ir_baralho(Jogo* jogo, int jogador __attribute__((unused))) {
	if (jogo->partida_finalizada) return false;
	if (jogo->aguardando_resposta_truco || jogo->aguardando_resposta_envido ||
	    jogo->aguardando_resposta_flor) return false;

	return true;
}

void ir_baralho(Jogo* jogo, int jogador) {
	// Jogador desiste - oponente ganha os pontos atuais
	int oponente = (jogador == 1) ? 2 : 1;

	if (oponente == 1) {
		jogo->pontos_jogador1 += jogo->valor_rodada;
	} else {
		jogo->pontos_jogador2 += jogo->valor_rodada;
	}

	if (!verificar_fim_partida(jogo)) {
		nova_mao(jogo);
	}
}

bool verificar_fim_partida(Jogo* jogo) {
	if (jogo->pontos_jogador1 >= PONTOS_VITORIA) {
		jogo->partida_finalizada = true;
		jogo->vencedor_partida = 1;
		return true;
	}

	if (jogo->pontos_jogador2 >= PONTOS_VITORIA) {
		jogo->partida_finalizada = true;
		jogo->vencedor_partida = 2;
		return true;
	}

	return false;
}

EstadoJogo obter_estado_jogo(Jogo* jogo, int jogador) {
	EstadoJogo estado;
	memset(&estado, 0, sizeof(EstadoJogo));

	// Transforma pontos para serem relativos ao jogador (sempre mostra "seus pontos" como jogador1)
	if (jogador == 1) {
		estado.pontos_jogador1 = jogo->pontos_jogador1;
		estado.pontos_jogador2 = jogo->pontos_jogador2;
	} else {
		estado.pontos_jogador1 = jogo->pontos_jogador2;  // Inverte para jogador 2
		estado.pontos_jogador2 = jogo->pontos_jogador1;
	}

	estado.rodada_atual = jogo->rodada_atual;
	// Converte mao_jogador para ser relativa (1 = você é mão, 2 = oponente é mão)
	estado.mao_jogador = (jogo->mao_jogador == jogador) ? 1 : 2;
	// Converte vez_jogador para ser relativa (1 = sua vez, 2 = vez do oponente)
	estado.vez_jogador = (jogo->vez_jogador == jogador) ? 1 : 2;
	estado.valor_rodada = jogo->valor_rodada;
	estado.valor_envido = jogo->valor_envido;
	estado.valor_flor = jogo->valor_flor;

	// Copia as cartas da mão do jogador
	Jogador* j = (jogador == 1) ? &jogo->jogador1 : &jogo->jogador2;
	estado.num_cartas_mao = j->num_cartas;
	for (int i = 0; i < j->num_cartas; i++) {
		estado.cartas_mao[i] = j->mao[i];
	}

	// Cartas jogadas em TODAS as rodadas (permanecem visíveis até fim da mão)
	// Inicializa com naipe 0 (vazio)
	memset(estado.cartas_jogadas_rodada, 0, sizeof(estado.cartas_jogadas_rodada));

	// Preenche cartas de todas as rodadas já jogadas
	// Para cada jogador, índice par = sua carta, índice ímpar = carta do oponente
	for (int r = 0; r < 3; r++) {
		Rodada* rodada = &jogo->rodadas[r];
		// Índices: rodada 0 -> [0,1], rodada 1 -> [2,3], rodada 2 -> [4,5]
		// Índice par = carta do jogador que está vendo, ímpar = carta do oponente
		if (jogador == 1) {
			if (rodada->jogador1_jogou) {
				estado.cartas_jogadas_rodada[r * 2] = rodada->carta_jogador1;
			}
			if (rodada->jogador2_jogou) {
				estado.cartas_jogadas_rodada[r * 2 + 1] = rodada->carta_jogador2;
			}
		} else {
			// Para jogador 2, inverte: sua carta no índice par, oponente no ímpar
			if (rodada->jogador2_jogou) {
				estado.cartas_jogadas_rodada[r * 2] = rodada->carta_jogador2;
			}
			if (rodada->jogador1_jogou) {
				estado.cartas_jogadas_rodada[r * 2 + 1] = rodada->carta_jogador1;
			}
		}
	}

	estado.pode_cantar_truco = pode_cantar_truco(jogo, jogador);
	estado.pode_cantar_envido = pode_cantar_envido(jogo, jogador);
	estado.pode_cantar_flor = pode_cantar_flor(jogo, jogador);

	// Aguardando resposta apenas se o OPONENTE cantou/aumentou (você precisa responder)
	// Usa ultimo_a_aumentar ao invés de jogador_cantou pois Retruco/Vale4 mudam jogador_cantou
	bool oponente_cantou_truco = jogo->aguardando_resposta_truco && jogo->ultimo_a_aumentar_truco != jogador;
	bool oponente_cantou_envido = jogo->aguardando_resposta_envido && jogo->ultimo_a_aumentar_envido != jogador;
	bool oponente_cantou_flor = jogo->aguardando_resposta_flor && jogo->jogador_cantou_flor != jogador;
	estado.aguardando_resposta = oponente_cantou_truco || oponente_cantou_envido || oponente_cantou_flor;

	return estado;
}
