#include "common.h"

#include <stdio.h>

const char* naipe_para_string(Naipe naipe) {
	switch (naipe) {
		case NAIPE_ESPADAS:
			return "Espadas";
		case NAIPE_PAUS:
			return "Paus";
		case NAIPE_COPAS:
			return "Copas";
		case NAIPE_OUROS:
			return "Ouros";
		default:
			return "Desconhecido";
	}
}

const char* numero_para_string(NumeroCarta numero) {
	switch (numero) {
		case NUMERO_AS:
			return "As";
		case NUMERO_2:
			return "2";
		case NUMERO_3:
			return "3";
		case NUMERO_4:
			return "4";
		case NUMERO_5:
			return "5";
		case NUMERO_6:
			return "6";
		case NUMERO_7:
			return "7";
		case NUMERO_10:
			return "10";
		case NUMERO_11:
			return "11";
		case NUMERO_12:
			return "12";
		default:
			return "?";
	}
}

void imprimir_carta(Carta carta) {
	printf("%s de %s", numero_para_string(carta.numero), naipe_para_string(carta.naipe));
}
