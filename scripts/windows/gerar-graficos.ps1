# Gera os graficos e as tabelas LaTeX dentro do container (servico 'graficos').
#
# Uso:
#   .\gerar-graficos.ps1
#
# Requer resultados\*.csv ja existentes (rode antes .\gerar-testes.ps1).
# Saida: graficos\*.png e graficos\tabelas.tex no host.

. "$PSScriptRoot\_comum.ps1"

$csvs = Get-ChildItem -Path (Join-Path $script:Raiz 'resultados') -Filter '*.csv' -ErrorAction SilentlyContinue
if (-not $csvs) {
    Write-Error 'Nao ha CSVs em resultados\. Rode .\gerar-testes.ps1 antes.'
    exit 1
}

Write-Titulo 'Graficos e tabelas no container'
Invoke-Rodar -Servico 'graficos'
Write-Host 'OK: figuras em graficos\.'
