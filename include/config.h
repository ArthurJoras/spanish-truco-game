// Configuração do Cliente Gráfico - Truco Espanhol
// Edite este arquivo e recompile para aplicar as mudanças

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// CONFIGURAÇÕES DE REDE
// ============================================
#define CONFIG_IP_PADRAO "127.0.0.1"
#define CONFIG_PORTA_PADRAO 8888
#define CONFIG_TIMEOUT_CONEXAO 5  // segundos

// ============================================
// CONFIGURAÇÕES DE JANELA
// ============================================
#define CONFIG_LARGURA_JANELA 1280
#define CONFIG_ALTURA_JANELA 720
#define CONFIG_TITULO_JANELA "Truco Espanhol"
#define CONFIG_TELA_CHEIA false

// ============================================
// CONFIGURAÇÕES DE RENDERIZAÇÃO
// ============================================
#define CONFIG_FPS_ALVO 60
#define CONFIG_VSYNC true
#define CONFIG_ANTIALIASING true

// ============================================
// CONFIGURAÇÕES DE CARTAS
// ============================================
#define CONFIG_LARGURA_CARTA 100
#define CONFIG_ALTURA_CARTA 150
#define CONFIG_ESPACAMENTO_CARTA 20
#define CONFIG_ELEVACAO_HOVER 10      // pixels
#define CONFIG_ELEVACAO_SELECAO 20    // pixels
#define CONFIG_ANIMACAO_CARTAS false  // Para implementação futura

// ============================================
// CONFIGURAÇÕES DE CORES (RGB)
// ============================================

// Fundo (Verde mesa de jogo)
#define CONFIG_COR_FUNDO_R 34
#define CONFIG_COR_FUNDO_G 139
#define CONFIG_COR_FUNDO_B 34

// Botões
#define CONFIG_COR_BOTAO_R 139
#define CONFIG_COR_BOTAO_G 69
#define CONFIG_COR_BOTAO_B 19

#define CONFIG_COR_BOTAO_HOVER_R 160
#define CONFIG_COR_BOTAO_HOVER_G 82
#define CONFIG_COR_BOTAO_HOVER_B 45

// Texto
#define CONFIG_COR_TEXTO_R 255
#define CONFIG_COR_TEXTO_G 255
#define CONFIG_COR_TEXTO_B 255

// Destaque
#define CONFIG_COR_DESTAQUE_R 255
#define CONFIG_COR_DESTAQUE_G 255
#define CONFIG_COR_DESTAQUE_B 0

// ============================================
// CONFIGURAÇÕES DE FONTES
// ============================================
#define CONFIG_TAMANHO_FONTE_TITULO 48
#define CONFIG_TAMANHO_FONTE_NORMAL 24
#define CONFIG_TAMANHO_FONTE_PEQUENA 18

// Caminhos de fontes (em ordem de prioridade)
#define CONFIG_FONTES_SISTEMA {                                     \
	"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",         \
	"/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf", \
	"/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",          \
	NULL}

// ============================================
// CONFIGURAÇÕES DE ASSETS
// ============================================
#define CONFIG_PASTA_IMAGENS "img/"
#define CONFIG_FORMATO_CARTA "%02d-%s.png"
#define CONFIG_ARQUIVO_VERSO "reverso.png"

// ============================================
// CONFIGURAÇÕES DE INTERFACE
// ============================================
#define CONFIG_DURACAO_MENSAGEM_TEMPORARIA 3.0f  // segundos
#define CONFIG_MOSTRAR_FPS false
#define CONFIG_MOSTRAR_DEBUG false

// Posições de elementos
#define CONFIG_MARGEM_TELA 20
#define CONFIG_ESPACAMENTO_BOTOES 70

// ============================================
// CONFIGURAÇÕES DE SOM (FUTURO)
// ============================================
#define CONFIG_SOM_ATIVADO false
#define CONFIG_VOLUME_MASTER 70   // 0-100
#define CONFIG_VOLUME_EFEITOS 80  // 0-100
#define CONFIG_VOLUME_MUSICA 50   // 0-100

// ============================================
// CONFIGURAÇÕES AVANÇADAS
// ============================================
#define CONFIG_MAX_BOTOES 20
#define CONFIG_BUFFER_MENSAGENS 10
#define CONFIG_LOG_NIVEL 1  // 0=None, 1=Error, 2=Warning, 3=Info, 4=Debug

// ============================================
// TEMAS PREDEFINIDOS
// ============================================

// Descomente para usar um tema diferente:

// TEMA ESCURO
/*
#undef CONFIG_COR_FUNDO_R
#undef CONFIG_COR_FUNDO_G
#undef CONFIG_COR_FUNDO_B
#define CONFIG_COR_FUNDO_R 20
#define CONFIG_COR_FUNDO_G 60
#define CONFIG_COR_FUNDO_B 20
*/

// TEMA AZUL (Mesa de poker)
/*
#undef CONFIG_COR_FUNDO_R
#undef CONFIG_COR_FUNDO_G
#undef CONFIG_COR_FUNDO_B
#define CONFIG_COR_FUNDO_R 0
#define CONFIG_COR_FUNDO_G 100
#define CONFIG_COR_FUNDO_B 0
*/

// TEMA VERMELHO (Mesa de cassino)
/*
#undef CONFIG_COR_FUNDO_R
#undef CONFIG_COR_FUNDO_G
#undef CONFIG_COR_FUNDO_B
#define CONFIG_COR_FUNDO_R 139
#define CONFIG_COR_FUNDO_G 0
#define CONFIG_COR_FUNDO_B 0
*/

#endif  // CONFIG_H

/*
 * NOTA: Após editar este arquivo, recompile o projeto:
 *   make clean
 *   make cliente_grafico
 *
 * DICAS:
 *   - Para tela cheia, mude CONFIG_TELA_CHEIA para true
 *   - Para mudar resolução, ajuste CONFIG_LARGURA_JANELA e CONFIG_ALTURA_JANELA
 *   - Para cartas maiores, aumente CONFIG_LARGURA_CARTA e CONFIG_ALTURA_CARTA
 *   - Para debug, ative CONFIG_MOSTRAR_DEBUG
 */
