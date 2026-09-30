#!/usr/bin/env bash
# Gera os graficos e as tabelas LaTeX dentro do container (servico 'graficos').
#
# Uso:
#   ./gerar-graficos.sh
#
# Requer resultados/*.csv ja existentes (rode antes ./gerar-testes.sh).
# Saida: graficos/*.png e graficos/tabelas.tex no host.

source "$(dirname "${BASH_SOURCE[0]}")/_comum.sh"

if ! ls resultados/*.csv >/dev/null 2>&1; then
    echo "AVISO: nao ha CSVs em resultados/." >&2
    echo "Rode ./gerar-testes.sh antes para produzir os dados." >&2
    exit 1
fi

titulo "Graficos e tabelas no container"
rodar graficos
echo "OK: figuras em graficos/."
