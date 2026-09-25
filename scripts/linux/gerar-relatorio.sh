#!/usr/bin/env bash
# Compila o relatorio LaTeX dentro do container (servico 'relatorio').
#
# Uso:
#   ./gerar-relatorio.sh
#
# Saida: relatorio/Projeto.pdf no host. As figuras usadas pelo relatorio sao
# copiadas manualmente de graficos/ para relatorio/imagem/ (nao e automatico).

source "$(dirname "${BASH_SOURCE[0]}")/_comum.sh"

titulo "Relatorio LaTeX no container"
rodar relatorio
echo "OK: relatorio/Projeto.pdf."
