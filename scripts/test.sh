#!/bin/bash

# Script para testar o jogo de truco espanhol

echo "=========================================="
echo "   TESTE DO JOGO DE TRUCO ESPANHOL"
echo "=========================================="
echo ""

# Verificar se está no Linux/WSL
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "⚠️  Este script deve ser executado no Linux ou WSL"
    echo "   Se você está no Windows, abra o WSL e execute novamente"
    exit 1
fi

# Limpar compilações anteriores
echo "🧹 Limpando arquivos anteriores..."
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
echo "=========================================="
echo "   INSTRUÇÕES PARA TESTAR"
echo "=========================================="
echo ""
echo "1️⃣  Em um terminal, execute:"
echo "   ./servidor"
echo ""
echo "2️⃣  Em outro terminal, execute:"
echo "   ./cliente"
echo ""
echo "3️⃣  Em um terceiro terminal, execute:"
echo "   ./cliente"
echo ""
echo "4️⃣  No primeiro cliente:"
echo "   - Escolha opção 1 (Criar sala)"
echo "   - Digite um nome para a sala"
echo ""
echo "5️⃣  No segundo cliente:"
echo "   - Escolha opção 2 (Listar salas)"
echo "   - Escolha opção 3 (Entrar em sala)"
echo "   - Digite o ID da sala criada"
echo ""
echo "6️⃣  Em qualquer um dos clientes:"
echo "   - Escolha opção 4 (Iniciar partida)"
echo ""
echo "7️⃣  Joguem! Use:"
echo "   - Opção 5 para jogar carta"
echo "   - Opção 6 para cantar Truco/Envido/Flor"
echo ""
echo "=========================================="
echo ""
echo "Deseja iniciar o servidor agora? (s/n)"
read -r resposta

if [[ "$resposta" == "s" ]] || [[ "$resposta" == "S" ]]; then
    echo ""
    echo "🚀 Iniciando servidor..."
    echo "   Pressione Ctrl+C para parar"
    echo ""
    ./servidor
else
    echo ""
    echo "Para iniciar manualmente, use:"
    echo "  ./servidor           # Porta padrão 8888"
    echo "  ./servidor 9000      # Porta personalizada"
    echo ""
    echo "Para iniciar cliente:"
    echo "  ./cliente            # Conecta a localhost:8888"
    echo "  ./cliente 127.0.0.1 9000  # IP e porta personalizados"
fi
