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

$zipPackages = Get-ChildItem -Path $RootDir -Filter "*.zip" -File -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -like "$BuildPath*" -or $_.DirectoryName -eq $RootDir }
$exePackages = Get-ChildItem -Path $RootDir -Filter "*.exe" -File -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -like "$BuildPath*" -or $_.DirectoryName -eq $RootDir }

if ($exePackages.Count -eq 0) {
    throw "No NSIS installer found for build path $BuildPath"
}

Write-Host "Windows package verification passed."
Write-Host "Executable: $ExePath"
Write-Host "Plugin directory: $PluginPath"
Write-Host "EXE packages:"
$exePackages | ForEach-Object { Write-Host " - $($_.FullName)" }

if ($zipPackages.Count -eq 0) {
    Write-Host "ZIP packages: none found"
} else {
    Write-Host "ZIP packages:"
    $zipPackages | ForEach-Object { Write-Host " - $($_.FullName)" }
}
