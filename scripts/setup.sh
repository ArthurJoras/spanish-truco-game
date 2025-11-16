#!/bin/bash

# Script para tornar todos os scripts executáveis

echo "🔧 Tornando scripts executáveis..."

chmod +x renomear_cartas.sh
chmod +x install_deps.sh
chmod +x start_gui.sh
chmod +x test.sh

echo "✅ Scripts prontos!"
echo ""
echo "Agora você pode executar:"
echo "  ./install_deps.sh    - Instala dependências"
echo "  ./renomear_cartas.sh - Renomeia cartas"
echo "  ./start_gui.sh       - Início rápido GUI"
echo "  ./test.sh            - Teste geral"
