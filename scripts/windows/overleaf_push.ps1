# Envia o conteudo de relatorio/ para um projeto do Overleaf via Git integration.
#
# PRE-REQUISITOS (uma unica vez, no site do Overleaf):
#   1. Crie o projeto (New Project -> Blank Project) e copie o ID da URL:
#      https://www.overleaf.com/project/<ID>
#   2. Abra Account Settings -> Git authentication tokens -> Generate token.
#   3. Defina as variaveis nesta sessao do PowerShell:
#        $env:OVERLEAF_PROJECT_ID = '<ID>'
#        $env:OVERLEAF_TOKEN      = '<TOKEN>'
#
# Uso:
#   .\scripts\windows\overleaf_push.ps1
#   .\scripts\windows\overleaf_push.ps1 -Mensagem "atualiza figuras"
#
# O script espelha relatorio/ na RAIZ do projeto Overleaf (mesma estrutura do
# ZIP). Nao grava o token em disco de forma permanente: usa um GIT_ASKPASS
# temporario, apagado ao final.

param(
    [string]$Mensagem = 'Atualiza relatorio (TP PAA Quicksort)'
)

$ErrorActionPreference = 'Stop'

$raiz = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
Set-Location $raiz

if (-not $env:OVERLEAF_PROJECT_ID) {
    Write-Error 'Defina $env:OVERLEAF_PROJECT_ID (ID do projeto, na URL .../project/<ID>).'
    exit 1
}
if (-not $env:OVERLEAF_TOKEN) {
    Write-Error 'Defina $env:OVERLEAF_TOKEN (Account Settings -> Git authentication tokens).'
    exit 1
}

$remoto = "https://git@git.overleaf.com/$($env:OVERLEAF_PROJECT_ID)"
$work = Join-Path $raiz '.overleaf-work'

# Autenticacao nao interativa: usuario "git" na URL, token via GIT_ASKPASS.
$ask = Join-Path $env:TEMP 'overleaf-askpass.cmd'
Set-Content -Path $ask -Encoding ascii -Value "@echo off`r`necho %OVERLEAF_TOKEN%"
$env:GIT_ASKPASS = $ask
$env:GIT_TERMINAL_PROMPT = '0'

if (Test-Path (Join-Path $work '.git')) {
    git -C $work remote set-url origin $remoto
    git -C $work fetch --quiet origin
    $ramo = git -C $work rev-parse --abbrev-ref HEAD
    git -C $work reset --hard "origin/$ramo"
} else {
    Write-Host 'Clonando o projeto do Overleaf...'
    git clone --quiet $remoto $work
}

# Espelha relatorio/ na raiz do projeto Overleaf (remove o que nao existe mais).
Get-ChildItem -Path $work -Force | Where-Object { $_.Name -ne '.git' } |
    Remove-Item -Recurse -Force
Get-ChildItem -Path (Join-Path $raiz 'relatorio') -Force |
    Copy-Item -Destination $work -Recurse -Force

# Remove intermediarios do LaTeX (mesma lista de scripts/overleaf_zip.py).
Get-ChildItem -Path $work -Recurse -File -Include `
    *.aux, *.bbl, *.blg, *.log, *.out, *.toc, *.lof, *.lot, *.loq, *.fls,
    *.fdb_latexmk, *.synctex.gz | Remove-Item -Force

Set-Location $work
git add -A
if (-not (git status --porcelain)) {
    Write-Host 'Nada mudou: o Overleaf ja esta atualizado.'
} else {
    $nome = if ($env:OVERLEAF_COMMIT_NAME) { $env:OVERLEAF_COMMIT_NAME } else { 'Overleaf Sync' }
    $email = if ($env:OVERLEAF_COMMIT_EMAIL) { $env:OVERLEAF_COMMIT_EMAIL } else { 'overleaf@local' }
    git -c user.name=$nome -c user.email=$email commit --quiet -m $Mensagem
    $ramo = git rev-parse --abbrev-ref HEAD
    git push --quiet origin "HEAD:$ramo"
    Write-Host 'OK: relatorio enviado ao Overleaf.'
}

Remove-Item $ask -Force -ErrorAction SilentlyContinue
