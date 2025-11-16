# 🃏 Truco Espanhol - Multiplayer

Um jogo de truco espanhol 1v1 implementado em C com arquitetura cliente-servidor, suportando múltiplas salas simultâneas e interface gráfica com SDL2.

<div align="center">

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![SDL2](https://img.shields.io/badge/SDL2-0080FF?style=for-the-badge&logo=sdl&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

</div>

## ✨ Características

- 🎮 **Multijogador 1v1**: Partidas de truco entre dois jogadores
- 🏠 **Múltiplas salas**: Suporte para várias partidas simultâneas
- 🔌 **TCP Sockets**: Comunicação robusta com protocolo binário
- 🎯 **Regras completas**: Truco, Envido, Flor, Retruco, Vale 4, Real Envido, Falta Envido
- 🖥️ **Interface gráfica**: Cliente SDL2 com cartas visuais e botões interativos
- 📟 **Interface terminal**: Cliente minimalista para testes
- 🧵 **Multithreading**: Servidor com pthread para múltiplas conexões simultâneas

## 📁 Estrutura do Projeto

```
trabalho_truco/
├── src/              # Código fonte (.c)
│   ├── servidor.c
│   ├── cliente.c
│   ├── cliente_grafico.c
│   ├── ui_grafica.c
│   ├── game_logic.c
│   └── common.c
├── include/          # Headers (.h)
│   ├── common.h
│   ├── game_logic.h
│   └── ui_grafica.h
├── build/            # Executáveis compilados
├── assets/           # Imagens das cartas (PNG)
│   └── img/
├── docs/             # Documentação adicional
├── scripts/          # Scripts auxiliares
├── Makefile
└── README.md
```

## 🎮 Regras do Jogo

O truco espanhol é jogado com baralho de 40 cartas (sem 8 e 9). Cada partida tem múltiplas mãos, e cada mão tem até 3 rodadas (melhor de 3).

### 🃏 Hierarquia das Cartas (da mais forte para a mais fraca)

1. **As de Espadas** 🗡️ (Espada Ancha)
2. **As de Paus** ♣️
3. **7 de Espadas** 🗡️
4. **7 de Ouros** ♦️
5. **Todos os 3s**
6. **Todos os 2s**
7. **Ases de Copas e Ouros**
8. **Reis (12)**
9. **Cavalos (11)**
10. **Valetes (10)**
11. **7 de Paus e Copas**
12. **6, 5, 4**

### 🎲 Cantos e Apostas

#### Truco (Aposta de Rodada)

- **Truco**: Aumenta valor de 1 → 2 pontos
- **Retruco**: Resposta aumentando para 3 pontos
- **Vale 4**: Resposta final aumentando para 4 pontos

#### Envido (Aposta de Cartas)

- **Envido**: 2 pontos base
- **Real Envido**: +3 pontos adicionais
- **Falta Envido**: Pontos restantes para 15

#### Flor

- Quando tem 3 cartas do mesmo naipe

### 🏆 Vitória

Primeiro jogador a atingir **15 pontos** vence a partida.

## 🚀 Início Rápido

### 1️⃣ Instalar Dependências

```bash
# Ubuntu/Debian/WSL
make install-deps

# Ou manualmente:
sudo apt-get install gcc make libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### 2️⃣ Compilar

```bash
make
```

### 3️⃣ Executar

**Terminal 1 - Servidor:**

```bash
make run-server
```

**Terminal 2 - Cliente 1:**

```bash
make run-client-gui
```

**Terminal 3 - Cliente 2:**

```bash
make run-client-gui
```

## 🛠️ Comandos do Makefile

| Comando               | Descrição                            |
| --------------------- | ------------------------------------ |
| `make` ou `make all`  | Compila tudo                         |
| `make run-server`     | Inicia o servidor                    |
| `make run-client`     | Inicia cliente terminal              |
| `make run-client-gui` | Inicia cliente gráfico               |
| `make demo`           | Inicia servidor em background        |
| `make stop-server`    | Para servidor em background          |
| `make clean`          | Remove arquivos compilados           |
| `make install-deps`   | Instala dependências (Ubuntu/Debian) |
| `make help`           | Mostra ajuda completa                |

## 🎯 Como Jogar

### Cliente Gráfico

1. **Menu Principal:**

   - `Criar Sala`: Cria nova sala de jogo
   - `Listar Salas`: Mostra salas disponíveis
   - `Sair`: Fecha o cliente

2. **No Lobby:**

   - Aguarde outro jogador entrar
   - Clique em `Iniciar Partida` quando ambos estiverem conectados

3. **Durante a Partida:**
   - **Selecione uma carta** clicando nela
   - **Jogar Carta**: Joga a carta selecionada
   - **TRUCO!**: Canta truco (aumenta aposta)
   - **Envido**: Canta envido (aposta de cartas)
   - **Quero/Não Quero**: Aceita ou recusa o canto do oponente
   - **Retruco/Vale Quatro**: Aumenta a aposta após Truco
   - **Real Envido/Falta Envido**: Aumenta a aposta após Envido

### Cliente Terminal

Interface de texto com comandos numerados. Basta digitar o número da opção desejada.

## 🔧 Configuração Avançada

### Mudar Porta do Servidor

```bash
cd build
./servidor 9000
```

### Conectar a Servidor Remoto

```bash
cd build
./cliente_grafico 192.168.1.100 9000
```

## 📊 Arquitetura Técnica

### Protocolo de Comunicação

- **Estrutura**: Mensagens binárias de 520 bytes
- **18 tipos de mensagens**: Conexão, sala, partida, jogadas, cantos
- **Thread-safe**: Mutex para proteção de dados compartilhados

### Servidor

- **Multithreaded**: pthread para cada cliente
- **Gestão de salas**: Suporte para múltiplas partidas simultâneas
- **Broadcast**: Notificações em tempo real para ambos os jogadores

### Cliente Gráfico

- **SDL2**: Renderização de cartas e interface
- **60 FPS**: Loop de renderização suave
- **Event-driven**: Resposta a cliques e eventos de rede

## 🐛 Troubleshooting

### Erro ao carregar cartas

Certifique-se de que a pasta `assets/img/` contém todas as 40 cartas no formato:

```
01-espadas.png, 02-espadas.png, ..., 12-espadas.png
01-paus.png, 02-paus.png, ..., 12-paus.png
01-copas.png, 02-copas.png, ..., 12-copas.png
01-ouros.png, 02-ouros.png, ..., 12-ouros.png
```

### Erro de compilação SDL2

```bash
# Reinstale as dependências
sudo apt-get install --reinstall libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Servidor não inicia

Verifique se a porta 8888 está disponível:

```bash
lsof -i :8888
```

## 📚 Documentação Adicional

Consulte a pasta `docs/` para documentação detalhada sobre:

- Implementação das regras
- Guia da interface gráfica
- Checklist de funcionalidades

## 🤝 Contribuindo

Este projeto foi desenvolvido como trabalho acadêmico para a disciplina de Redes de Computadores.

## 📝 Licença

Projeto acadêmico - Universidade Federal do ABC (UFABC)

## 👥 Autores

Desenvolvido como trabalho da disciplina de Redes de Computadores.

---

<div align="center">

**🎮 Bom jogo! Que ganhe o melhor! 🏆**

</div>
