# Utilitarios comuns aos scripts de Windows (PowerShell). NAO executar sozinho.
#
# Uso nos demais scripts:
#   . "$PSScriptRoot\_comum.ps1"
#
# Depende apenas de Docker Desktop (docker compose). Nao requer make, g++ nem
# Python no host: o repositorio e montado em /app pelo docker-compose.yml.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Raiz do repositorio: este arquivo vive em scripts\windows\.
$script:Raiz = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
Set-Location $script:Raiz

# O docker-compose.yml usa ${UID:-1000}:${GID:-1000}. No Windows nao existe UID
# numerico; 1000 e o padrao do compose e funciona com o bind mount do Docker
# Desktop. Defina as variaveis se precisar de outro valor.
if (-not $env:UID) { $env:UID = '1000' }
if (-not $env:GID) { $env:GID = '1000' }

# Corte M para o Insertion Sort. Sobrescreva:  $env:M=25; .\gerar-testes.ps1
if (-not $env:M) { $env:M = '40' }
$script:M = $env:M

function Invoke-Rodar {
    param(
        [Parameter(Mandatory = $true)][string]$Servico,
        [string[]]$Argumentos = @()
    )
    & docker compose run --rm --build --no-deps $Servico @Argumentos
    if ($LASTEXITCODE -ne 0) {
        throw "docker compose run $Servico falhou (codigo $LASTEXITCODE)."
    }
}

function Write-Titulo {
    param([Parameter(Mandatory = $true)][string]$Texto)
    Write-Host ''
    Write-Host ('=' * 60)
    Write-Host " $Texto"
    Write-Host ('=' * 60)
}
