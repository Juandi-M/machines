param(
    [string]$RuntimeDirectory = (Join-Path $PSScriptRoot '../build/opengl/runtime-baseline/machines')
)

$ErrorActionPreference = 'Stop'
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$source = Join-Path $runtime 'libvorbisfile-3.dll'
$destination = Join-Path $runtime 'vorbisfile.dll'

# ALURE 1.2 dynamically loads this exact filename on Windows.
foreach ($name in @('libvorbisfile-3.dll', 'libvorbis-0.dll', 'libogg-0.dll')) {
    if (-not (Test-Path -LiteralPath (Join-Path $runtime $name) -PathType Leaf)) {
        throw "Missing music dependency: $name in $runtime"
    }
}

$sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
if (Test-Path -LiteralPath $destination) {
    if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -ne $sourceHash) {
        throw "Existing vorbisfile.dll differs from libvorbisfile-3.dll; inspect it before replacing it."
    }
} else {
    Copy-Item -LiteralPath $source -Destination $destination
}

Write-Output "Prepared ALURE music decoder: $destination"
Write-Output "SHA256: $sourceHash"
