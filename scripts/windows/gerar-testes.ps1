# Gera os CSVs de teste/experimento dentro do container (servico 'experimentos').
#
# Uso:
#   .\gerar-testes.ps1              # make tudo (validar + calibrar + experimentos + pior-caso)
#   .\gerar-testes.ps1 validar      # so a validacao de corretude
#   .\gerar-testes.ps1 calibrar     # so a calibracao de M
#   .\gerar-testes.ps1 experimentos # so a bateria principal
#   .\gerar-testes.ps1 pior-caso    # so o pior caso forcado
#   $env:M=25; .\gerar-testes.ps1   # sobrescreve o corte M (padrao: 40)
#
# Saida: resultados\*.csv no host.

. "$PSScriptRoot\_comum.ps1"

$alvo = if ($args.Count -ge 1) { $args[0] } else { 'tudo' }
$validos = @('validar', 'calibrar', 'experimentos', 'pior-caso', 'tudo')
if ($validos -notcontains $alvo) {
    Write-Error "Alvo invalido: '$alvo'. Use: $($validos -join ' | ')"
    exit 1
}

Write-Titulo "Testes e experimentos (make $alvo M=$script:M) no container"
Invoke-Rodar -Servico 'experimentos' -Argumentos @($alvo, "M=$script:M")
Write-Host 'OK: CSVs em resultados\.'
