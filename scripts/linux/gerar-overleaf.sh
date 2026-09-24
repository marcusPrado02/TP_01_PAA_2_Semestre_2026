#!/usr/bin/env bash
# Empacota o relatorio em relatorio-overleaf.zip (upload no Overleaf).
#
# Uso:
#   ./gerar-overleaf.sh
#
# Saida: relatorio-overleaf.zip na raiz do projeto. O pacote e autocontido;
# no Overleaf basta definir Projeto.tex como documento principal (pdfLaTeX).

source "$(dirname "${BASH_SOURCE[0]}")/_comum.sh"

titulo "Pacote Overleaf no container"
rodar overleaf
echo "OK: relatorio-overleaf.zip."
