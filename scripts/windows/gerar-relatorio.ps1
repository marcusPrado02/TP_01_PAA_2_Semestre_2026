# Compila o relatorio LaTeX dentro do container (servico 'relatorio').
#
# Uso:
#   .\gerar-relatorio.ps1
#
# Saida: relatorio\Projeto.pdf no host. As figuras usadas pelo relatorio sao
# copiadas manualmente de graficos\ para relatorio\imagem\ (nao e automatico).

. "$PSScriptRoot\_comum.ps1"

Write-Titulo 'Relatorio LaTeX no container'
Invoke-Rodar -Servico 'relatorio'
Write-Host 'OK: relatorio\Projeto.pdf.'
