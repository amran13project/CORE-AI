$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

function Resolve-Executable([string]$Name, [string[]]$Candidates) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    return $null
}

$cmake = Resolve-Executable 'cmake' @(
    (Join-Path ${env:ProgramFiles} 'CMake\bin\cmake.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'CMake\bin\cmake.exe'),
    (Join-Path ${env:LOCALAPPDATA} 'Programs\CMake\bin\cmake.exe'),
    (Join-Path ${env:USERPROFILE} 'scoop\apps\cmake\current\bin\cmake.exe'),
    (Join-Path ${env:ChocolateyToolsLocation} 'cmake\current\bin\cmake.exe'),
    'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
)

if (-not $cmake) {
    Write-Host ''
    Write-Host 'CORE AI: CMake was not found.' -ForegroundColor Yellow
    Write-Host 'Install the free official CMake package, then reopen PowerShell.'
    Write-Host 'Official download: https://cmake.org/download/'
    Write-Host ''
    throw 'CMake is required to build CORE AI.'
}

$cmakeDir = Split-Path -Parent $cmake
$env:PATH = "$cmakeDir;$env:PATH"
Write-Host "[CORE] CMake: $cmake"
& $cmake --version
if ($LASTEXITCODE -ne 0) { throw 'CMake could not be executed.' }

if (-not (Test-Path '.\core.config')) {
    Copy-Item '.\core.config.example' '.\core.config'
}

$ninja = Resolve-Executable 'ninja' @(
    (Join-Path ${env:ProgramFiles} 'Ninja\ninja.exe'),
    (Join-Path ${env:USERPROFILE} 'scoop\apps\ninja\current\ninja.exe'),
    (Join-Path ${env:ChocolateyToolsLocation} 'ninja\current\ninja.exe')
)

$useNinja = [bool]$ninja
if ($useNinja) {
    $ninjaDir = Split-Path -Parent $ninja
    $env:PATH = "$ninjaDir;$env:PATH"
    Write-Host "[CORE] Ninja: $ninja"
    & $ninja --version
} else {
    Write-Host '[CORE] Ninja not found; using CMake default generator.' -ForegroundColor Yellow
}

if ($useNinja) {
    & $cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }

    & $cmake --build build
    if ($LASTEXITCODE -ne 0) { throw 'CORE AI build failed.' }

    & $cmake --build build --target test -- -V
    if ($LASTEXITCODE -ne 0) {
        ctest --test-dir build --output-on-failure
        if ($LASTEXITCODE -ne 0) { throw 'CORE AI tests failed.' }
    }
} else {
    & $cmake -S . -B build
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }

    & $cmake --build build --config Release
    if ($LASTEXITCODE -ne 0) { throw 'CORE AI build failed.' }

    & ctest --test-dir build -C Release --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'CORE AI tests failed.' }
}

Write-Host ''
Write-Host 'CORE AI build and tests completed successfully.' -ForegroundColor Green
Write-Host 'Executable: .\build\core-ai.exe'
Write-Host 'Diagnostics: .\build\core-ai.exe doctor'
Write-Host 'Capabilities: .\build\core-ai.exe capabilities'
Write-Host ''

$ollama = Get-Command ollama -ErrorAction SilentlyContinue
if ($ollama) {
    Write-Host "[CORE] Ollama detected: $($ollama.Source)" -ForegroundColor Green
} else {
    Write-Warning 'Ollama is not on PATH. CORE AI can still build, but local model chat needs a configured provider.'
}
