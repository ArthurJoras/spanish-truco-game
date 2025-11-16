# Cliente Gráfico - Truco Espanhol

Interface gráfica desenvolvida com SDL2 para o jogo de truco espanhol.

## 📦 Instalação de Dependências

### Ubuntu/Debian/WSL

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Fedora/RHEL

```bash
sudo dnf install SDL2-devel SDL2_image-devel SDL2_ttf-devel
```

### Arch Linux

```bash
sudo pacman -S sdl2 sdl2_image sdl2_ttf
```

## 🎨 Preparação dos Assets

Antes de compilar, renomeie as imagens das cartas de espanhol para português:

```bash
chmod +x renomear_cartas.sh
./renomear_cartas.sh
```

Isso irá renomear:

- `bastos` → `paus`
- `oros` → `ouros`
- `copas` e `espadas` permanecem iguais

## 🔨 Compilação

```bash
# Compilar apenas o cliente gráfico
make cliente_grafico

# Ou compilar tudo (servidor + clientes)
make
```

## 🚀 Execução

```bash
# Conectar ao servidor local
./cliente_grafico

# Conectar a um servidor remoto
./cliente_grafico 192.168.1.100

# Conectar com porta personalizada
./cliente_grafico 192.168.1.100 9000
```

Ou usando o Makefile:

```bash
make run-client-gui
```

## 🎮 Controles

### Mouse

- **Clique nos botões**: Executar ações
- **Clique nas cartas**: Selecionar carta para jogar
- **Hover nas cartas**: Destacar carta

### Teclado

- **ESC**: Fechar aplicação
- **Digite texto**: Ao criar sala

## 🖥️ Telas do Jogo

### 1. Menu Principal

- **Criar Sala**: Cria uma nova sala de jogo
- **Listar Salas**: Mostra salas disponíveis
- **Sair**: Fecha o cliente

### 2. Criar Sala

- Digite o nome da sala
- Clique em "Criar" para confirmar
- Aguarde outro jogador

### 3. Listar Salas

- Veja todas as salas disponíveis
- Clique em "Entrar" na sala desejada
- Salas em jogo mostram "[EM JOGO]"

### 4. Lobby

- Aguarde o segundo jogador
- Clique em "Iniciar Partida" quando ambos estiverem prontos

### 5. Jogo

**Área Superior:**

- Placar (Você x Oponente)
- Valor da rodada atual
- Indicador de mão (quem começa)
- Indicador de vez

**Área Central:**

- Cartas do oponente (viradas)
- Mesa com cartas jogadas
- Mensagens temporárias

**Área Inferior:**

- Suas cartas (clicáveis)
- Carta selecionada fica elevada

**Área Direita - Botões:**

- **Jogar Carta**: Joga a carta selecionada
- **TRUCO!**: Canta truco (se disponível)
- **Envido**: Canta envido (se disponível)
- **Flor**: Canta flor (se disponível)

**Ao receber um canto:**

- **Quero**: Aceita o desafio
- **Não Quero**: Recusa e concede pontos

### 6. Fim de Partida

- Mostra o vencedor
- Botão para voltar ao menu

## 🎨 Características Visuais

### Cores

- **Fundo**: Verde mesa (34, 139, 34)
- **Botões**: Marrom madeira (139, 69, 19)
- **Hover**: Marrom claro (160, 82, 45)
- **Texto**: Branco
- **Destaque**: Amarelo

### Cartas

- **Dimensões**: 100x150 pixels
- **Selecionada**: Borda amarela dupla, elevada 20px
- **Hover**: Borda amarela simples, elevada 10px
- **Espaçamento**: 20px entre cartas

### Layout

- **Resolução**: 1280x720 pixels
- **Janela**: Centralizada
- **FPS**: 60 (com VSync)

## 🐛 Resolução de Problemas

### Erro: "cannot find -lSDL2"

Instale as bibliotecas SDL2:

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Erro ao carregar fontes

O cliente tenta carregar fontes do sistema em:

- `/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf`
- `/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf`
- `/usr/share/fonts/truetype/freefont/FreeSansBold.ttf`

Instale uma delas:

```bash
sudo apt-get install fonts-dejavu-core
```

### Erro ao carregar imagens das cartas

Certifique-se de que:

1. A pasta `img/` existe no diretório do executável
2. As cartas foram renomeadas corretamente (`./renomear_cartas.sh`)
3. Os arquivos têm permissão de leitura

### Performance baixa

- Verifique se tem aceleração de hardware: `glxinfo | grep "direct rendering"`
- Atualize drivers da placa de vídeo
- Feche outros programas que usam GPU

## 🔧 Configuração Avançada

### Modificar Resolução

Edite `ui_grafica.h`:

```c
#define LARGURA_JANELA 1920  // Nova largura
#define ALTURA_JANELA 1080   // Nova altura
```

### Modificar Cores

Edite as constantes em `ui_grafica.h`:

```c
#define COR_FUNDO_R 34
#define COR_FUNDO_G 139
#define COR_FUNDO_B 34
```

### Adicionar Sons (Futuro)

Adicione SDL_mixer:

```bash
sudo apt-get install libsdl2-mixer-dev
```

## 📊 Recursos Utilizados

- **SDL2**: Janela e renderização
- **SDL2_image**: Carregamento de PNG
- **SDL2_ttf**: Renderização de texto
- **pthreads**: Thread de rede assíncrona

## 🎯 Melhorias Futuras

- [ ] Animações de cartas
- [ ] Efeitos sonoros
- [ ] Música de fundo
- [ ] Transições suaves entre telas
- [ ] Avatares dos jogadores
- [ ] Chat visual
- [ ] Histórico de jogadas
- [ ] Replay das partidas
- [ ] Temas customizáveis
- [ ] Suporte a tela cheia
- [ ] Múltiplas resoluções

## 📝 Arquitetura do Código

```
cliente_grafico.c
├── Gerenciamento de conexão (sockets)
├── Thread de recebimento de mensagens
├── Callbacks de botões
├── Loop principal (60 FPS)
└── Sincronização com mutex

ui_grafica.c
├── Inicialização SDL2
├── Carregamento de assets
├── Renderização por tela
├── Gerenciamento de botões
├── Desenho de cartas
└── Processamento de eventos

ui_grafica.h
├── Estruturas de dados
├── Enums de telas
└── Protótipos
```

## 🤝 Integração com Servidor

O cliente gráfico usa o mesmo protocolo do cliente terminal:

- Mensagens binárias via TCP
- Estrutura `Mensagem` compartilhada
- Thread assíncrona para não bloquear UI
- Mutex para thread-safety

---

**Divirta-se jogando truco! 🎴✨**
