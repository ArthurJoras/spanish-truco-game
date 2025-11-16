#!/bin/bash

# Script de instalação de dependências para Truco Espanhol

echo "=========================================="
echo "  INSTALADOR DE DEPENDENCIAS - TRUCO    "
echo "=========================================="
echo ""

# Detectar sistema operacional
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "❌ Não foi possível detectar o sistema operacional"
    exit 1
fi

echo "Sistema detectado: $OS"
echo ""

# Função de instalação para Ubuntu/Debian
install_ubuntu() {
    echo "📦 Instalando dependências para Ubuntu/Debian..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        gcc \
        make \
        libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-ttf-dev \
        fonts-dejavu-core \
        pkg-config
    
    if [ $? -eq 0 ]; then
        echo "✅ Dependências instaladas com sucesso!"
        return 0
    else
        echo "❌ Erro ao instalar dependências"
        return 1
    fi
}

# Função de instalação para Fedora/RHEL
install_fedora() {
    echo "📦 Instalando dependências para Fedora/RHEL..."
    sudo dnf install -y \
        gcc \
        make \
        SDL2-devel \
        SDL2_image-devel \
        SDL2_ttf-devel \
        dejavu-sans-fonts \
        pkg-config
    
    if [ $? -eq 0 ]; then
        echo "✅ Dependências instaladas com sucesso!"
        return 0
    else
        echo "❌ Erro ao instalar dependências"
        return 1
    fi
}

# Função de instalação para Arch Linux
install_arch() {
    echo "📦 Instalando dependências para Arch Linux..."
    sudo pacman -S --noconfirm \
        gcc \
        make \
        sdl2 \
        sdl2_image \
        sdl2_ttf \
        ttf-dejavu \
        pkg-config
    
    if [ $? -eq 0 ]; then
        echo "✅ Dependências instaladas com sucesso!"
        return 0
    else
        echo "❌ Erro ao instalar dependências"
        return 1
    fi
}

# Selecionar instalador apropriado
case $OS in
    ubuntu|debian|linuxmint)
        install_ubuntu
        ;;
    fedora|rhel|centos)
        install_fedora
        ;;
    arch|manjaro)
        install_arch
        ;;
    *)
        echo "⚠️  Sistema '$OS' não suportado automaticamente"
        echo ""
        echo "Instale manualmente:"
        echo "  - GCC"
        echo "  - Make"
        echo "  - SDL2"
        echo "  - SDL2_image"
        echo "  - SDL2_ttf"
        echo "  - Fontes DejaVu"
        exit 1
        ;;
esac

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  VERIFICANDO INSTALAÇÃO"
    echo "=========================================="
    echo ""
    
    # Verificar GCC
    if command -v gcc &> /dev/null; then
        echo "✅ GCC: $(gcc --version | head -n1)"
    else
        echo "❌ GCC não encontrado"
    fi
    
    # Verificar Make
    if command -v make &> /dev/null; then
        echo "✅ Make: $(make --version | head -n1)"
    else
        echo "❌ Make não encontrado"
    fi
    
    # Verificar SDL2
    if pkg-config --exists sdl2; then
        echo "✅ SDL2: $(pkg-config --modversion sdl2)"
    else
        echo "❌ SDL2 não encontrado"
    fi
    
    # Verificar SDL2_image
    if pkg-config --exists SDL2_image; then
        echo "✅ SDL2_image: $(pkg-config --modversion SDL2_image)"
    else
        echo "❌ SDL2_image não encontrado"
    fi
    
    # Verificar SDL2_ttf
    if pkg-config --exists SDL2_ttf; then
        echo "✅ SDL2_ttf: $(pkg-config --modversion SDL2_ttf)"
    else
        echo "❌ SDL2_ttf não encontrado"
    fi
    
    echo ""
    echo "=========================================="
    echo "  PRÓXIMOS PASSOS"
    echo "=========================================="
    echo ""
    echo "1. Renomear cartas:"
    echo "   ./renomear_cartas.sh"
    echo ""
    echo "2. Compilar projeto:"
    echo "   make"
    echo ""
    echo "3. Executar servidor:"
    echo "   ./servidor"
    echo ""
    echo "4. Executar cliente gráfico (outro terminal):"
    echo "   ./cliente_grafico"
    echo ""
    echo "Ou use o script de início rápido:"
    echo "   ./start_gui.sh"
    echo ""
fi
