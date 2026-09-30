#!/usr/bin/env bash
# Utilitarios comuns aos scripts de Linux. NAO executar diretamente.
#
# Uso nos demais scripts:
#   source "$(dirname "${BASH_SOURCE[0]}")/_comum.sh"
#
# Nao depende de make nem de g++/Python: apenas docker compose. O repositorio e
# montado em /app pelo docker-compose.yml.

set -euo pipefail

# Raiz do repositorio: este arquivo vive em scripts/linux/.
RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$RAIZ"

# O docker-compose.yml usa ${UID:-1000}:${GID:-1000} para os arquivos gerados
# sairem com o dono do host. UID e readonly no bash: nao pode ser ATRIBUIDO, mas
# pode ser reexportado (ja contem o uid do usuario). GID normalmente nao existe.
export UID
export GID="${GID:-$(id -g)}"

# Valor de M (corte para o Insertion Sort) repassado ao alvo 'make'. Sobrescreva
# com a variavel de ambiente M:  M=25 ./gerar-testes.sh
M="${M:-40}"

# Executa um servico do compose de forma descartavel, construindo a imagem se
# necessario. --no-deps evita disparar dependencias (ex.: rodar os graficos sem
# reexecutar os experimentos).
rodar() {
    local servico="$1"
    shift
    docker compose run --rm --build --no-deps "$servico" "$@"
}

titulo() {
    printf '\n============================================================\n'
    printf ' %s\n' "$1"
    printf '============================================================\n'
}
