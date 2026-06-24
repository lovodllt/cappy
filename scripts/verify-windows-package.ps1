param(
    [string]$BuildDir = "build-win"
)

$ErrorActionPreference = "Stop"

$RootDir = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $RootDir $BuildDir
$ExePath = Join-Path $BuildPath "bin/cappy.exe"
$PluginPath = Join-Path $BuildPath "bin/plugins"

if (-not (Test-Path $ExePath)) {
    throw "Missing executable: $ExePath"
}

if (-not (Test-Path $PluginPath)) {
    throw "Missing plugin directory: $PluginPath"
}

$zipPackages = Get-ChildItem -Path $BuildPath -Filter "*.zip" -File -ErrorAction SilentlyContinue
$exePackages = Get-ChildItem -Path $BuildPath -Filter "*.exe" -File -ErrorAction SilentlyContinue

if ($zipPackages.Count -eq 0) {
    throw "No ZIP package found in $BuildPath"
}

if ($exePackages.Count -eq 0) {
    throw "No NSIS installer found in $BuildPath"
}

Write-Host "Windows package verification passed."
Write-Host "Executable: $ExePath"
Write-Host "Plugin directory: $PluginPath"
Write-Host "ZIP packages:"
$zipPackages | ForEach-Object { Write-Host " - $($_.FullName)" }
Write-Host "EXE packages:"
$exePackages | ForEach-Object { Write-Host " - $($_.FullName)" }
