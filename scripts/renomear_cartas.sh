#!/bin/bash

# Script para renomear cartas de espanhol para português
# bastos -> paus
# oros -> ouros
# copas -> copas (mantém)
# espadas -> espadas (mantém)

cd img

echo "Renomeando cartas de espanhol para português..."

# Renomear bastos para paus
for arquivo in *-bastos.png; do
    novo_nome="${arquivo/bastos/paus}"
    mv "$arquivo" "$novo_nome"
    echo "  $arquivo -> $novo_nome"
done

# Renomear oros para ouros
for arquivo in *-oros.png; do
    novo_nome="${arquivo/oros/ouros}"
    mv "$arquivo" "$novo_nome"
    echo "  $arquivo -> $novo_nome"
done

echo "Renomeação concluída!"
echo ""
echo "Estrutura final:"
ls -1 *.png | head -20
