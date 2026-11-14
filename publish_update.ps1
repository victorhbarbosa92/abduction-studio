<#
.SYNOPSIS
    Script de Publicação em 1 Clique para o Abduction Studio V2.
    Empacota o patch de atualização e cria uma Release Privada no GitHub.

.EXAMPLE
    .\publish_update.ps1 -Version "2.0.1" -Changelog "Correção do sync de waveform e novos efeitos"
#>

param (
    [Parameter(Mandatory=$true, HelpMessage="Versão do patch (ex: 2.0.1)")]
    [string]$Version,

    [Parameter(Mandatory=$false)]
    [string]$Changelog = "Correções de estabilidade e melhorias no motor de áudio.",

    [Parameter(Mandatory=$false)]
    [string]$GitHubOwner = "",

    [Parameter(Mandatory=$false)]
    [string]$GitHubRepo = "",

    [Parameter(Mandatory=$false)]
    [string]$GitHubToken = ""
)

$ErrorActionPreference = "Stop"

# Garante prefixo sem 'v' para consistência
$cleanVersion = $Version.TrimStart('v', 'V')
$tagName = "v$cleanVersion"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "🚀 ABDUCTION STUDIO - PUBLICADOR DE ATUALIZAÇÕES (HOTFIX)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "Versão alvo: $tagName" -ForegroundColor Yellow

$rootDir = $PSScriptRoot
$destExe = Join-Path $rootDir "AbductionStudioV2.exe"
$updaterExe = Join-Path $rootDir "AbductionUpdater.exe"
$updaterConfig = Join-Path $rootDir "updater.json"

if (-not (Test-Path $destExe)) {
    Write-Error "ERRO: O executável principal $destExe não foi encontrado!"
    exit 1
}

# 1. Garante que o AbductionUpdater.exe existe
if (-not (Test-Path $updaterExe)) {
    Write-Host "Compilando AbductionUpdater.exe..." -ForegroundColor DarkYellow
    $updaterSrc = Join-Path $rootDir "updater\src\AbductionUpdater.cpp"
    & g++ -O2 -s -mwindows "$updaterSrc" -o "$updaterExe" -lcomctl32 -lshell32
    if (-not (Test-Path $updaterExe)) {
        Write-Error "Falha ao compilar AbductionUpdater.exe."
        exit 1
    }
}

# 2. Prepara pasta de staging para o Patch
$tempDir = [System.IO.Path]::GetTempPath()
$stagingDir = Join-Path $tempDir "AbductionStudio_Patch_v$cleanVersion"
$zipOutput = Join-Path $rootDir "AbductionStudio_Patch_v$cleanVersion.zip"

if (Test-Path $stagingDir) { Remove-Item $stagingDir -Recurse -Force }
if (Test-Path $zipOutput)  { Remove-Item $zipOutput -Force }

New-Item -ItemType Directory -Path $stagingDir | Out-Null

Write-Host "Empacotando arquivos essenciais para o patch..." -ForegroundColor Green

# Copia arquivos do patch
Copy-Item $destExe -Destination (Join-Path $stagingDir "AbductionStudioV2.exe") -Force
Copy-Item $updaterExe -Destination (Join-Path $stagingDir "AbductionUpdater.exe") -Force

if (Test-Path $updaterConfig) {
    try {
        $jsonObj = Get-Content $updaterConfig -Raw | ConvertFrom-Json
        $jsonObj.version = $cleanVersion
        $jsonObj | ConvertTo-Json -Depth 4 | Set-Content $updaterConfig -Encoding UTF8
    } catch {}
    Copy-Item $updaterConfig -Destination (Join-Path $stagingDir "updater.json") -Force
}

# Copia DLLs essenciais se existirem na raiz
Get-ChildItem -Path $rootDir -Filter "*.dll" | ForEach-Object {
    Copy-Item $_.FullName -Destination $stagingDir -Force
}

# 3. Gera o arquivo .zip com tar.exe nativo (super rápido)
Write-Host "Compactando em $zipOutput..." -ForegroundColor Green
$prevPwd = Get-Location
Set-Location $stagingDir
& tar.exe -a -c -f $zipOutput *
Set-Location $prevPwd

# Limpa o staging
Remove-Item $stagingDir -Recurse -Force

$zipItem = Get-Item $zipOutput
$zipSizeMb = [math]::Round($zipItem.Length / 1MB, 2)
$hash = (Get-FileHash -Path $zipOutput -Algorithm SHA256).Hash

Write-Host "✅ Pacote criado com sucesso!" -ForegroundColor Green
Write-Host "   Arquivo: $($zipItem.Name)" -ForegroundColor White
Write-Host "   Tamanho: $zipSizeMb MB" -ForegroundColor White
Write-Host "   SHA256:  $hash" -ForegroundColor Gray

# 4. Configuração das credenciais do GitHub
$configFile = Join-Path $rootDir "updater\github_config.json"
$config = @{}

if (Test-Path $configFile) {
    try {
        $config = Get-Content $configFile -Raw | ConvertFrom-Json -AsHashtable
    } catch {}
}

if (-not $GitHubOwner) {
    $GitHubOwner = if ($config.owner) { $config.owner } else { Read-Host "Informe seu usuário/organização do GitHub" }
}
if (-not $GitHubRepo) {
    $GitHubRepo = if ($config.repo) { $config.repo } else { Read-Host "Informe o nome do repositório privado" }
}
if (-not $GitHubToken) {
    $GitHubToken = if ($env:GITHUB_TOKEN) { 
        $env:GITHUB_TOKEN 
    } elseif ($config.token) { 
        $config.token 
    } else { 
        Read-Host "Informe seu GitHub Personal Access Token (PAT)" 
    }
}

# Salva configuração para não precisar digitar novamente (exceto token se preferir)
$saveConfig = @{
    owner = $GitHubOwner
    repo  = $GitHubRepo
    token = $GitHubToken
}
$saveConfig | ConvertTo-Json | Set-Content $configFile -Encoding UTF8

if (-not $GitHubToken) {
    Write-Host "`n⚠️ Nenhum token do GitHub informado." -ForegroundColor Yellow
    Write-Host "O arquivo de patch foi gerado em: $zipOutput" -ForegroundColor White
    Write-Host "Você pode fazer o upload manualmente na aba Releases do seu repositório no GitHub." -ForegroundColor White
    exit 0
}

# 5. Criação da Release Privada via API do GitHub
Write-Host "`nPublicando Release $tagName no GitHub ($GitHubOwner/$GitHubRepo)..." -ForegroundColor Cyan

$ghHeaders = @{
    "Authorization" = "Bearer $GitHubToken"
    "Accept"        = "application/vnd.github.v3+json"
    "User-Agent"    = "Abduction-Publisher"
}

$releaseBody = @{
    tag_name         = $tagName
    name             = "Abduction Studio $tagName"
    body             = $Changelog
    draft            = $false
    prerelease       = $false
} | ConvertTo-Json

try {
    $releaseUrl = "https://api.github.com/repos/$GitHubOwner/$GitHubRepo/releases"
    $createRes = Invoke-RestMethod -Uri $releaseUrl -Method Post -Headers $ghHeaders -Body $releaseBody -ContentType "application/json; charset=utf-8"
    $releaseId = $createRes.id
    Write-Host "Release criada com ID: $releaseId" -ForegroundColor Green
} catch {
    # Se a release já existir com a mesma tag, tenta buscar o ID
    if ($_.Exception.Response.StatusCode.value__ -eq 422) {
        Write-Host "Tag $tagName já existe. Buscando release existente..." -ForegroundColor Yellow
        $getReleaseUrl = "https://api.github.com/repos/$GitHubOwner/$GitHubRepo/releases/tags/$tagName"
        $createRes = Invoke-RestMethod -Uri $getReleaseUrl -Method Get -Headers $ghHeaders
        $releaseId = $createRes.id
    } else {
        Write-Error "Falha ao criar release: $($_.Exception.Message)"
        exit 1
    }
}

# 6. Upload do Asset (Patch .zip)
Write-Host "Enviando arquivo $zipOutput para o GitHub..." -ForegroundColor Cyan
$uploadUrl = "https://uploads.github.com/repos/$GitHubOwner/$GitHubRepo/releases/$releaseId/assets?name=$($zipItem.Name)"

$uploadHeaders = @{
    "Authorization" = "Bearer $GitHubToken"
    "Accept"        = "application/vnd.github.v3+json"
    "Content-Type"  = "application/zip"
    "User-Agent"    = "Abduction-Publisher"
}

$zipBytes = [System.IO.File]::ReadAllBytes($zipOutput)
try {
    $uploadRes = Invoke-RestMethod -Uri $uploadUrl -Method Post -Headers $uploadHeaders -Body $zipBytes
    Write-Host "==========================================================" -ForegroundColor Green
    Write-Host "🎉 SUCESSO! ATUALIZAÇÃO $tagName PUBLICADA COM SUCESSO!" -ForegroundColor Green
    Write-Host "==========================================================" -ForegroundColor Green
    Write-Host "Assim que os usuários abrirem o Abduction Studio nas outras" -ForegroundColor White
    Write-Host "máquinas, o aviso de atualização aparecerá automaticamente!" -ForegroundColor White
} catch {
    Write-Error "Falha no envio do asset: $($_.Exception.Message)"
}
