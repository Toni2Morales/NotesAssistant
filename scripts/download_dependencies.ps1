
$ErrorActionPreference = "Stop"

# -----------------------------------------
# NotesAssistant - Dependency Downloader
# -----------------------------------------

$ProjectRoot = Split-Path -Parent $PSScriptRoot

$ExternalDir = Join-Path $ProjectRoot "external/sherpa-onnx"
$ModelsDir = Join-Path $ProjectRoot "models/spanish-kws"
$TempDir = Join-Path $env:TEMP "NotesAssistant-Dependencies"

New-Item -ItemType Directory -Force -Path $ExternalDir | Out-Null
New-Item -ItemType Directory -Force -Path $ModelsDir | Out-Null
New-Item -ItemType Directory -Force -Path $TempDir | Out-Null

function Download-File {
    param(
        [string]$Url,
        [string]$Destination
    )

    if (Test-Path $Destination) {
        Write-Host "Ya existe: $Destination"
        return
    }

    Write-Host "Descargando: $([System.IO.Path]::GetFileName($Destination))"

    Invoke-WebRequest -Uri $Url -OutFile $Destination

    if (!(Test-Path $Destination) -or
        (Get-Item $Destination).Length -eq 0) {
        throw "La descarga ha fallado: $Url"
    }
}

# -----------------------------------------
# 1. Descargar sherpa-onnx v2.0.1
# -----------------------------------------

$SherpaVersion = "v2.0.1"

$SherpaArchiveName = "sherpa-onnx-v2.0.1-win-x64-shared-MT-Release-no-tts.tar.bz2"

$SherpaUrl = "https://github.com/k2-fsa/sherpa-onnx/releases/download/$SherpaVersion/$SherpaArchiveName"

$SherpaArchive = Join-Path $TempDir $SherpaArchiveName

$SherpaHeader = Join-Path $ExternalDir "include/sherpa-onnx/c-api/cxx-api.h"

if (!(Test-Path $SherpaHeader)) {

    Download-File -Url $SherpaUrl -Destination $SherpaArchive

    Write-Host "Descomprimiendo sherpa-onnx..."

    $ExtractDir = Join-Path $TempDir "sherpa-extracted"

    New-Item -ItemType Directory -Force -Path $ExtractDir | Out-Null

    tar.exe -xjf $SherpaArchive -C $ExtractDir

    if ($LASTEXITCODE -ne 0) {
        throw "No se pudo descomprimir sherpa-onnx."
    }

    $Header = Get-ChildItem -Path $ExtractDir `
        -Filter "cxx-api.h" -File -Recurse |
        Select-Object -First 1

    if ($null -eq $Header) {
        throw "No se encontro cxx-api.h en el paquete."
    }

    $IncludeDir = $Header.Directory.Parent.Parent.Parent.FullName
    $PackageDir = Split-Path -Parent $IncludeDir

    foreach ($Folder in @("include", "lib", "bin")) {

        $Source = Join-Path $PackageDir $Folder
        $Destination = Join-Path $ExternalDir $Folder

        if (!(Test-Path $Source)) {
            throw "No se encontro la carpeta $Folder en sherpa-onnx."
        }

        Copy-Item -Path $Source -Destination $ExternalDir -Recurse -Force
    }

    Write-Host "sherpa-onnx instalado correctamente."
}
else {
    Write-Host "sherpa-onnx ya esta instalado."
}

# -----------------------------------------
# 2. Descargar modelo KWS en español
# -----------------------------------------

$ModelBaseUrl = "https://huggingface.co/jguerrisi/sherpa-onnx-kws-zipformer-es-3M-2026-08-08/resolve/main"

$ModelFiles = @(
    "encoder-epoch-20-avg-2-chunk-16-left-64.onnx",
    "decoder-epoch-20-avg-2-chunk-16-left-64.onnx",
    "joiner-epoch-20-avg-2-chunk-16-left-64.onnx",
    "tokens.txt",
    "bpe.model",
    "keywords.txt"
)

foreach ($File in $ModelFiles) {

    $Url = "$ModelBaseUrl/$File"
    $Destination = Join-Path $ModelsDir $File

    Download-File -Url $Url -Destination $Destination
}

# -----------------------------------------
# 3. Comprobacion final
# -----------------------------------------


$RequiredFiles = @(
    "external/sherpa-onnx/include/sherpa-onnx/c-api/cxx-api.h",
    "external/sherpa-onnx/lib/sherpa-onnx-cxx-api.lib",
    "external/sherpa-onnx/lib/sherpa-onnx-cxx-api.dll",
    "models/spanish-kws/tokens.txt",
    "models/spanish-kws/bpe.model",
    "models/spanish-kws/keywords.txt"
)


foreach ($File in $RequiredFiles) {

    $Path = Join-Path $ProjectRoot $File

    if (!(Test-Path $Path)) {
        throw "Falta un archivo necesario: $File"
    }
}

Write-Host ""
Write-Host "======================================"
Write-Host " NotesAssistant - Dependencias listas"
Write-Host "======================================"
Write-Host ""
Write-Host "Ya puedes configurar y compilar el proyecto."
