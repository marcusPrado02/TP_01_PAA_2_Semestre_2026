#!/usr/bin/env bash
# Gera os CSVs de teste/experimento dentro do container (servico 'experimentos').
#
# Uso:
#   ./gerar-testes.sh              # make tudo (validar + calibrar + experimentos + pior-caso)
#   ./gerar-testes.sh validar      # so a validacao de corretude
#   ./gerar-testes.sh calibrar     # so a calibracao de M
#   ./gerar-testes.sh experimentos # so a bateria principal
#   ./gerar-testes.sh pior-caso    # so o pior caso forcado
#   M=25 ./gerar-testes.sh         # sobrescreve o corte M (padrao: 40)
#
# Saida: resultados/*.csv no host.

source "$(dirname "${BASH_SOURCE[0]}")/_comum.sh"

ALVO="${1:-tudo}"
case "$ALVO" in
    validar|calibrar|experimentos|pior-caso|tudo) ;;
    *)
        echo "Alvo invalido: '$ALVO'." >&2
        echo "Use: validar | calibrar | experimentos | pior-caso | tudo" >&2
        exit 1
        ;;
esac

titulo "Testes e experimentos (make $ALVO M=$M) no container"
rodar experimentos "$ALVO" "M=$M"
echo "OK: CSVs em resultados/."
