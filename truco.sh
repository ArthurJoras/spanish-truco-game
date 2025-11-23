#!/bin/bash
# Script de início rápido para o Truco Espanhol

clear

echo "=========================================="
echo "  🃏 Truco Espanhol - Início Rápido"
echo "=========================================="
echo ""

# Verifica se está compilado
if [ ! -f "build/servidor" ] || [ ! -f "build/cliente_grafico" ]; then
    echo "⚙️  Compilando projeto..."
    make
    echo ""
fi

echo "Escolha uma opção:"
echo "  1) Iniciar Servidor"
echo "  2) Iniciar Cliente Gráfico"
echo "  3) Demo (Servidor em background)"
echo "  4) Parar Servidor"
echo "  5) Recompilar tudo"
echo "  0) Sair"
echo ""
read -p "Opção: " opcao

case $opcao in
    1)
        echo "🚀 Iniciando servidor na porta 8888..."
        ./build/servidor
        ;;
    2)
        echo "🎮 Iniciando cliente gráfico..."
        ./build/cliente_grafico
        ;;
    3)
        echo "🎬 Iniciando demo..."
        make demo
        ;;
    4)
        echo "🛑 Parando servidor..."
        make stop-server
        ;;
    5)
        echo "🔨 Recompilando..."
        make clean && make
        ;;
    0)
        echo "👋 Até logo!"
        exit 0
        ;;
    *)
        echo "❌ Opção inválida!"
        exit 1
        ;;
esac
