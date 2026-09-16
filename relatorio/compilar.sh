#!/usr/bin/env bash
# Compila o relatorio (pdflatex -> bibtex -> pdflatex x2, para resolver citacoes).
set -euo pipefail
cd "$(dirname "$0")"
DOC=Projeto

pdflatex -interaction=nonstopmode -halt-on-error "$DOC".tex
bibtex "$DOC" || true          # bibtex avisa sobre entradas nao usadas; nao e fatal
pdflatex -interaction=nonstopmode -halt-on-error "$DOC".tex
pdflatex -interaction=nonstopmode -halt-on-error "$DOC".tex

echo
echo "OK: $(pwd)/$DOC.pdf"
grep -c "^" "$DOC".log >/dev/null
if grep -qE "Citation .* undefined|Reference .* undefined" "$DOC".log; then
    echo "AVISO: ha citacoes ou referencias nao resolvidas:"
    grep -E "Citation .* undefined|Reference .* undefined" "$DOC".log | sort -u
fi
