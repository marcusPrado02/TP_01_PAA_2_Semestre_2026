# Executa o fluxo completo, passo a passo, em containers:
#   1. testes e experimentos  -> resultados\*.csv
#   2. graficos e tabelas     -> graficos\*.png, graficos\tabelas.tex
#   3. copia das figuras      -> relatorio\imagem\
#   4. compilacao do relatorio-> relatorio\Projeto.pdf
#
# Uso:
#   .\fluxo-completo.ps1           # sequencial, sem pausa (padrao)
#   .\fluxo-completo.ps1 -Pausar   # pausa entre os passos (Enter para continuar)
#   $env:M=25; .\fluxo-completo.ps1   # sobrescreve o corte M (padrao: 40)
#
# O passo 3 e um Copy-Item de graficos\*.png para relatorio\imagem\: o servico
# de graficos NAO copia as figuras (ver AGENTS.md).

param(
    [switch]$Pausar
)

. "$PSScriptRoot\_comum.ps1"

function Wait-Pausa {
    if ($Pausar) {
        Write-Host ''
        Read-Host '-- Pressione Enter para continuar (ou Ctrl+C para parar)'
    }
}

Write-Titulo "FLUXO COMPLETO (M=$script:M)"
Write-Host 'Passos: 1) experimentos  2) graficos  3) copiar figuras  4) relatorio'
Wait-Pausa

Write-Titulo 'Passo 1/4 - Testes e experimentos'
Invoke-Rodar -Servico 'experimentos' -Argumentos @('tudo', "M=$script:M")
Wait-Pausa

Write-Titulo 'Passo 2/4 - Graficos e tabelas'
Invoke-Rodar -Servico 'graficos'
Wait-Pausa

Write-Titulo 'Passo 3/4 - Copiar figuras para relatorio\imagem\'
$origem = Join-Path $script:Raiz 'graficos'
$destino = Join-Path $script:Raiz 'relatorio\imagem'
$figuras = Get-ChildItem -Path $origem -Filter '*.png' -ErrorAction SilentlyContinue
if ($figuras) {
    Copy-Item -Path $figuras.FullName -Destination $destino -Force -Verbose
} else {
    Write-Warning 'Nenhuma figura em graficos\ para copiar.'
}
Wait-Pausa

Write-Titulo 'Passo 4/4 - Compilar o relatorio'
Invoke-Rodar -Servico 'relatorio'

Write-Titulo 'FLUXO CONCLUIDO'
Write-Host 'Saidas: resultados\*.csv | graficos\*.png | relatorio\Projeto.pdf'
