param(
    [string]$BuildDir = "build-win",
    [string]$QtBinDir = "",
    [string]$CMakeGenerator = "Ninja"
)

$ErrorActionPreference = "Stop"

$RootDir = Split-Path -Parent $PSScriptRoot
$BuildPath = Join-Path $RootDir $BuildDir

cmake -S $RootDir -B $BuildPath -G $CMakeGenerator -DCMAKE_BUILD_TYPE=Release
cmake --build $BuildPath --config Release

$ExePath = Join-Path $BuildPath "bin/cappy.exe"
if (-not (Test-Path $ExePath)) {
    throw "Missing executable: $ExePath"
}

$windeployqt = "windeployqt"
if ($QtBinDir -ne "") {
    $candidate = Join-Path $QtBinDir "windeployqt.exe"
    if (-not (Test-Path $candidate)) {
        throw "windeployqt.exe not found in QtBinDir: $QtBinDir"
    }
    $windeployqt = $candidate
}

& $windeployqt --release $ExePath
cpack --config (Join-Path $BuildPath "CPackConfig.cmake") -G ZIP
cpack --config (Join-Path $BuildPath "CPackConfig.cmake") -G NSIS
& (Join-Path $PSScriptRoot "verify-windows-package.ps1") -BuildDir $BuildDir

Write-Host "Windows packaging completed for $BuildPath"
