# Empacota o relatorio em relatorio-overleaf.zip (upload no Overleaf).
#
# Uso:
#   .\gerar-overleaf.ps1
#
# Saida: relatorio-overleaf.zip na raiz do projeto. O pacote e autocontido;
# no Overleaf basta definir Projeto.tex como documento principal (pdfLaTeX).

. "$PSScriptRoot\_comum.ps1"

Write-Titulo 'Pacote Overleaf no container'
Invoke-Rodar -Servico 'overleaf'
Write-Host 'OK: relatorio-overleaf.zip.'
