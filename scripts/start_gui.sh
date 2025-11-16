#!/bin/bash

# Script de início rápido para o Truco Espanhol Gráfico

echo "=============================================="
echo "  TRUCO ESPANHOL - INICIO RAPIDO (GRAFICO)  "
echo "=============================================="
echo ""

# Verificar sistema
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "⚠️  Este script deve ser executado no Linux ou WSL"
    exit 1
fi

# Verificar se SDL2 está instalado
echo "🔍 Verificando dependências..."

if ! pkg-config --exists sdl2; then
    echo "❌ SDL2 não encontrado"
    echo ""
    echo "Instalando dependências..."
    sudo apt-get update
    sudo apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev fonts-dejavu-core
else
    echo "✅ SDL2 encontrado"
fi

# Verificar se as cartas existem
if [ ! -d "img" ]; then
    echo "❌ Pasta img/ não encontrada!"
    echo "   Certifique-se de que a pasta com as imagens das cartas existe"
    exit 1
else
    echo "✅ Pasta img/ encontrada"
fi

# Renomear cartas se necessário
if ls img/*-bastos.png 1> /dev/null 2>&1; then
    echo "🔄 Renomeando cartas de espanhol para português..."
    chmod +x renomear_cartas.sh
    ./renomear_cartas.sh
else
    echo "✅ Cartas já renomeadas"
fi

# Limpar compilações anteriores
echo ""
echo "🧹 Limpando compilações anteriores..."
make clean > /dev/null 2>&1

# Compilar
echo "🔨 Compilando projeto..."
if make; then
    echo "✅ Compilação bem-sucedida!"
else
    echo "❌ Erro na compilação"
    exit 1
fi

echo ""
echo "=============================================="
echo "  PRONTO PARA JOGAR!"
echo "=============================================="
echo ""
echo "📋 Instruções:"
echo ""
echo "1️⃣  Terminal 1 - Servidor:"
echo "   ./servidor"
echo ""
echo "2️⃣  Terminal 2 - Cliente Gráfico 1:"
echo "   ./cliente_grafico"
echo ""
echo "3️⃣  Terminal 3 - Cliente Gráfico 2:"
echo "   ./cliente_grafico"
echo ""
echo "4️⃣  No Cliente 1:"
echo "   - Clique em 'Criar Sala'"
echo "   - Digite um nome"
echo "   - Clique em 'Criar'"
echo ""
echo "5️⃣  No Cliente 2:"
echo "   - Clique em 'Listar Salas'"
echo "   - Clique em 'Voltar'"
echo "   - Clique em 'Entrar em Sala' (se aparecer)"
echo "   - Digite o ID da sala"
echo ""
echo "6️⃣  Em qualquer cliente:"
echo "   - Clique em 'Iniciar Partida'"
echo ""
echo "7️⃣  Joguem! 🎮"
echo "   - Clique nas cartas para selecionar"
echo "   - Clique em 'Jogar Carta' para confirmar"
echo "   - Use os botões laterais para cantos"
echo ""
echo "=============================================="
echo ""
echo "Deseja iniciar o servidor agora? (s/n)"
read -r resposta

if [[ "$resposta" == "s" ]] || [[ "$resposta" == "S" ]]; then
    echo ""
    echo "🚀 Iniciando servidor..."
    echo "   Pressione Ctrl+C para parar"
    echo ""
    echo "   Em outros terminais, execute:"
    echo "   ./cliente_grafico"
    echo ""
    sleep 2
    ./servidor
else
    echo ""
    echo "Para iniciar manualmente:"
    echo "  ./servidor              # Terminal 1"
    echo "  ./cliente_grafico       # Terminal 2"
    echo "  ./cliente_grafico       # Terminal 3"
    echo ""
    echo "Ou simplifique com:"
    echo "  make demo               # Inicia servidor em background"
    echo "  make run-client-gui     # Cliente gráfico"
    echo ""
fi
