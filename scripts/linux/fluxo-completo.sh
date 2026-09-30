#!/usr/bin/env bash
# Executa o fluxo completo, passo a passo, em containers:
#   1. testes e experimentos  -> resultados/*.csv
#   2. graficos e tabelas     -> graficos/*.png, graficos/tabelas.tex
#   3. copia das figuras      -> relatorio/imagem/
#   4. compilacao do relatorio-> relatorio/Projeto.pdf
#
# Uso:
#   ./fluxo-completo.sh           # sequencial, sem pausa (padrao)
#   ./fluxo-completo.sh --pausar  # pausa entre os passos (Enter para continuar)
#   M=25 ./fluxo-completo.sh      # sobrescreve o corte M (padrao: 40)
#
# O passo 3 e um cp simples de graficos/*.png para relatorio/imagem/: o servico
# de graficos NAO copia as figuras (ver AGENTS.md).

source "$(dirname "${BASH_SOURCE[0]}")/_comum.sh"

PAUSA=0
for arg in "$@"; do
    case "$arg" in
        --pausar|--pausa|--step) PAUSA=1 ;;
        -h|--help)
            echo "Uso: $0 [--pausar]"
            echo "Fluxo: testes -> graficos -> copiar figuras -> relatorio"
            echo "Padrao: executa tudo de forma sequencial, sem pausas."
            exit 0
            ;;
        *) echo "Argumento desconhecido: $arg" >&2; exit 1 ;;
    esac
done

pausar() {
    if [ "$PAUSA" = "1" ]; then
        if [ -t 0 ]; then
            printf '\n-- Pressione Enter para continuar (ou Ctrl+C para parar)... '
            read -r _
        else
            echo "AVISO: --pausar ignorado (entrada nao interativa)." >&2
        fi
    fi
}

titulo "FLUXO COMPLETO (M=$M)"
echo "Passos: 1) experimentos  2) graficos  3) copiar figuras  4) relatorio"
pausar

titulo "Passo 1/4 - Testes e experimentos"
rodar experimentos tudo "M=$M"
pausar

titulo "Passo 2/4 - Graficos e tabelas"
rodar graficos
pausar

titulo "Passo 3/4 - Copiar figuras para relatorio/imagem/"
if ls graficos/*.png >/dev/null 2>&1; then
    cp -v graficos/*.png relatorio/imagem/
else
    echo "AVISO: nenhuma figura em graficos/ para copiar." >&2
fi
pausar

titulo "Passo 4/4 - Compilar o relatorio"
rodar relatorio

titulo "FLUXO CONCLUIDO"
echo "Saidas: resultados/*.csv | graficos/*.png | relatorio/Projeto.pdf"
