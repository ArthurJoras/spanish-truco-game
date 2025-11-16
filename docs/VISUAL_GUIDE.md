# 🎴 TRUCO ESPANHOL - GUIA VISUAL RÁPIDO

## 🚀 Início Rápido (3 passos)

```bash
# 1. Renomear cartas
./renomear_cartas.sh

# 2. Compilar
make

# 3. Jogar!
# Terminal 1:
./servidor

# Terminal 2:
./cliente_grafico

# Terminal 3:
./cliente_grafico
```

## 📁 Estrutura de Arquivos

```
trabalho_truco/
│
├── 🎮 SERVIDOR
│   ├── servidor.c          - Servidor multithreaded
│   ├── game_logic.c/h      - Regras do truco
│   └── common.c/h          - Protocolo compartilhado
│
├── 💻 CLIENTE TERMINAL
│   └── cliente.c           - Interface texto
│
├── 🎨 CLIENTE GRÁFICO
│   ├── cliente_grafico.c   - Main do cliente GUI
│   ├── ui_grafica.c/h      - Renderização SDL2
│   └── config.h            - Configurações
│
├── 🖼️ ASSETS
│   └── img/                - 40 cartas + verso
│
├── 🔧 BUILD & DOCS
│   ├── Makefile            - Sistema de build
│   ├── README.md           - Documentação principal
│   ├── README_GRAFICO.md   - Docs do cliente gráfico
│   └── GUIA_UI_GRAFICA.md  - Arquitetura técnica
│
└── 🚀 SCRIPTS
    ├── renomear_cartas.sh  - Renomeia assets
    ├── start_gui.sh        - Início rápido GUI
    └── test.sh             - Teste geral
```

## 🎯 Fluxo do Jogo

```
┌─────────────┐
│   SERVIDOR  │
│   (Porta    │
│    8888)    │
└──────┬──────┘
       │
       ├─────────────────────┐
       │                     │
┌──────▼──────┐      ┌──────▼──────┐
│  CLIENTE 1  │      │  CLIENTE 2  │
│  (Gráfico)  │      │  (Gráfico)  │
└─────────────┘      └─────────────┘
       │                     │
       ▼                     ▼
   Cria Sala            Entra Sala
       │                     │
       └──────────┬──────────┘
                  ▼
           Iniciar Partida
                  │
                  ▼
           ┌─────────────┐
           │ JOGANDO!    │
           │ - Cartas    │
           │ - Truco     │
           │ - Envido    │
           │ - Flor      │
           └─────────────┘
                  │
                  ▼
           Primeiro a 15
           pontos vence!
```

## 🎮 Controles do Cliente Gráfico

### 🖱️ Mouse

```
┌──────────────────────────────────────┐
│ CLIQUE ESQUERDO                      │
│  ├─ Botões    → Executar ação       │
│  ├─ Cartas    → Selecionar           │
│  └─ Menus     → Navegar              │
│                                       │
│ HOVER                                │
│  └─ Cartas    → Destaque visual      │
└──────────────────────────────────────┘
```

### ⌨️ Teclado

```
┌──────────────────────────────────────┐
│ ESC          → Fechar                │
│ BACKSPACE    → Apagar texto          │
│ LETRAS       → Digitar (criar sala)  │
└──────────────────────────────────────┘
```

## 📊 Layout da Tela de Jogo

```
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║  VOCÊ: 8 x 6 :OPO    Rodada: 2pts    [MÃO] >>> VEZ <<< ║
║                                                          ║
╠══════════════════════════════════════════════════════════╣
║                                                          ║
║              [🂠]  [🂠]  [🂠]                            ║
║           Cartas do Oponente (verso)                    ║
║                                                          ║
║  ┌────────────────────────────────────┐                 ║
║  │          MESA / CENTRO             │                 ║
║  │                                     │  [Jogar Carta]  ║
║  │    [🂡]        [🂱]                │  [TRUCO!]       ║
║  │  Carta J1    Carta J2              │  [Envido]       ║
║  │                                     │  [Flor]         ║
║  └────────────────────────────────────┘                 ║
║                                                          ║
║              [🃁]  [🃂]  [🃃]                           ║
║            Suas Cartas (clicáveis)                      ║
║         (destaque amarelo = selecionada)                ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
```

## 🎨 Paleta de Cores

```
┌─────────────────────────────────────────┐
│ ELEMENTO      COR            RGB        │
├─────────────────────────────────────────┤
│ Fundo Mesa    Verde Floresta (34,139,34)│
│ Botões        Marrom Madeira (139,69,19)│
│ Hover         Marrom Claro   (160,82,45)│
│ Texto         Branco         (255,255,255)│
│ Destaque      Amarelo        (255,255,0)│
│ Seleção       Amarelo Forte  (255,215,0)│
└─────────────────────────────────────────┘
```

## 🃏 Hierarquia das Cartas

```
MAIS FORTE ↓

1️⃣  As de Espadas  (Ancho) ⚔️
2️⃣  As de Paus               ♣
3️⃣  7 de Espadas            ⚔️
4️⃣  7 de Ouros              ♦
5️⃣  Todos os 3s
6️⃣  Todos os 2s
7️⃣  As de Copas/Ouros
8️⃣  Reis (12)
9️⃣  Cavalos (11)
🔟  Valetes (10)
1️⃣1️⃣  7 de Paus/Copas
1️⃣2️⃣  6, 5, 4

MAIS FRACO ↑
```

## 📈 Pontuação

```
CANTOS                  PONTOS
├─ Truco                  2
├─ Retruco                3
├─ Vale Quatro            4
├─ Envido                 2
├─ Real Envido            3
├─ Falta Envido           *
├─ Flor                   3
└─ Contraflor             6

* Falta Envido = pontos que faltam para 15

🏆 VITÓRIA = 15 PONTOS
```

## 🔧 Comandos Úteis

```bash
# Compilação
make                    # Tudo
make servidor           # Só servidor
make cliente_grafico    # Só GUI
make clean              # Limpar

# Execução
./servidor              # Servidor local
./servidor 9000         # Porta custom
./cliente_grafico       # Cliente local
./cliente_grafico IP    # Cliente remoto

# Helpers
make help               # Ajuda
make run-server         # Roda servidor
make run-client-gui     # Roda GUI
make demo               # Server background
make stop-server        # Para server
```

## 🐛 Troubleshooting

```
PROBLEMA                      SOLUÇÃO
─────────────────────────────────────────────────
❌ SDL2 não encontrado        sudo apt install libsdl2-dev
❌ Cartas não aparecem        ./renomear_cartas.sh
❌ Fonte não carrega          sudo apt install fonts-dejavu
❌ Servidor inacessível       Verificar firewall
❌ Lag/Travamento            Atualizar drivers GPU
❌ Erro ao compilar          make clean && make
```

## 📦 Dependências

```
SISTEMA OPERACIONAL
└─ Linux / WSL

COMPILADOR
└─ GCC

BIBLIOTECAS
├─ pthread     (threads)
├─ SDL2        (janela/render)
├─ SDL2_image  (PNG)
└─ SDL2_ttf    (fontes)

ASSETS
└─ 40 cartas PNG + reverso
```

## 🎓 Arquitetura Técnica

```
┌─────────────────────────────────────┐
│     CAMADA DE APRESENTAÇÃO          │
│  ┌─────────────────────────────┐   │
│  │    ui_grafica.c             │   │
│  │  - Renderização SDL2        │   │
│  │  - Eventos mouse/teclado    │   │
│  │  - Carregamento assets      │   │
│  └─────────────────────────────┘   │
├─────────────────────────────────────┤
│     CAMADA DE APLICAÇÃO             │
│  ┌─────────────────────────────┐   │
│  │  cliente_grafico.c          │   │
│  │  - Lógica cliente           │   │
│  │  - Gerenciamento estado     │   │
│  │  - Callbacks                │   │
│  └─────────────────────────────┘   │
├─────────────────────────────────────┤
│     CAMADA DE REDE                  │
│  ┌─────────────────────────────┐   │
│  │  common.c/h                 │   │
│  │  - Protocolo mensagens      │   │
│  │  - Sockets TCP              │   │
│  │  - Thread assíncrona        │   │
│  └─────────────────────────────┘   │
├─────────────────────────────────────┤
│     CAMADA DE LÓGICA                │
│  ┌─────────────────────────────┐   │
│  │  game_logic.c (servidor)    │   │
│  │  - Regras truco             │   │
│  │  - Validação jogadas        │   │
│  │  - Cálculo pontos           │   │
│  └─────────────────────────────┘   │
└─────────────────────────────────────┘
```

## 📚 Documentação

```
README.md              → Documentação geral
README_GRAFICO.md      → Guia cliente gráfico
GUIA_UI_GRAFICA.md     → Arquitetura técnica
VISUAL_GUIDE.md        → Este arquivo! 👋
config.h               → Configurações
```

## 🎉 Pronto para Jogar!

Execute o script de início rápido:

```bash
chmod +x start_gui.sh
./start_gui.sh
```

---

**Divirta-se! 🎴🎮🎉**

_Desenvolvido para disciplina de Redes de Computadores_
