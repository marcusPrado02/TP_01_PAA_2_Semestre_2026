#!/usr/bin/env bash
# Envia o conteudo de relatorio/ para um projeto do Overleaf via Git integration.
#
# PRE-REQUISITOS (uma unica vez, no site do Overleaf):
#   1. Crie o projeto (New Project -> Blank Project) e copie o ID da URL:
#      https://www.overleaf.com/project/<ID>
#   2. Abra Account Settings -> Git authentication tokens -> Generate token.
#   3. Exporte as variaveis nesta sessao:
#        export OVERLEAF_PROJECT_ID=<ID>
#        export OVERLEAF_TOKEN=<TOKEN>
#
# Uso:
#   ./scripts/linux/overleaf_push.sh
#   ./scripts/linux/overleaf_push.sh --mensagem "atualiza figuras"
#
# O script espelha relatorio/ na RAIZ do projeto Overleaf (mesma estrutura do
# ZIP), para que Projeto.tex seja o documento principal. Nao grava o token em
# disco nem o exibe; para isso usa GIT_ASKPASS.

set -euo pipefail

RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$RAIZ"

MENSAGEM="Atualiza relatorio (TP PAA Quicksort)"
if [ "${1:-}" = "--mensagem" ] && [ -n "${2:-}" ]; then
    MENSAGEM="$2"
fi

: "${OVERLEAF_PROJECT_ID:?Defina OVERLEAF_PROJECT_ID (ID do projeto, na URL .../project/<ID>)}"
: "${OVERLEAF_TOKEN:?Defina OVERLEAF_TOKEN (Account Settings -> Git authentication tokens)}"

REMOTO="https://git@git.overleaf.com/${OVERLEAF_PROJECT_ID}"
WORK="$RAIZ/.overleaf-work"
ASKPASS="$RAIZ/scripts/linux/_overleaf_askpass.sh"

# Autenticacao nao interativa: usuario "git" na URL, token via GIT_ASKPASS.
export OVERLEAF_USER="git"
export GIT_ASKPASS="$ASKPASS"
export GIT_TERMINAL_PROMPT=0

if [ -d "$WORK/.git" ]; then
    git -C "$WORK" remote set-url origin "$REMOTO"
    git -C "$WORK" fetch --quiet origin
    RAMO="$(git -C "$WORK" rev-parse --abbrev-ref HEAD)"
    git -C "$WORK" reset --hard "origin/$RAMO"
else
    echo "Clonando o projeto do Overleaf..."
    git clone --quiet "$REMOTO" "$WORK"
fi

# Espelha relatorio/ na raiz do projeto Overleaf (remove o que nao existe mais).
find "$WORK" -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} +
cp -a "$RAIZ/relatorio/." "$WORK/"

# Remove intermediarios do LaTeX (mesma lista de scripts/overleaf_zip.py).
find "$WORK" -type f \( \
    -name '*.aux' -o -name '*.bbl' -o -name '*.blg' -o -name '*.log' \
    -o -name '*.out' -o -name '*.toc' -o -name '*.lof' -o -name '*.lot' \
    -o -name '*.loq' -o -name '*.fls' -o -name '*.fdb_latexmk' \
    -o -name '*.synctex.gz' \) -delete

cd "$WORK"
git add -A
if git diff --cached --quiet; then
    echo "Nada mudou: o Overleaf ja esta atualizado."
else
    git -c user.name="${OVERLEAF_COMMIT_NAME:-Overleaf Sync}" \
        -c user.email="${OVERLEAF_COMMIT_EMAIL:-overleaf@local}" \
        commit --quiet -m "$MENSAGEM"
    RAMO="$(git rev-parse --abbrev-ref HEAD)"
    git push --quiet origin "HEAD:$RAMO"
    echo "OK: relatorio enviado ao Overleaf."
fi
