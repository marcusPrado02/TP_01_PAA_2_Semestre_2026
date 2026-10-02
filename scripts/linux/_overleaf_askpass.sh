#!/usr/bin/env bash
# Auxiliar de autenticacao do Git para o Overleaf (GIT_ASKPASS).
# NAO executar diretamente: o Git chama este script passando o texto do prompt
# em $1. Como o usuario "git" vai embutido na URL remota, o Git so pede a
# senha; devolvemos o token lido de OVERLEAF_TOKEN. O token nunca e exibido.
case "${1:-}" in
    *[Uu]sername*) printf '%s\n' "${OVERLEAF_USER:-git}" ;;
    *)             printf '%s\n' "${OVERLEAF_TOKEN:-}" ;;
esac
