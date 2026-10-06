$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')

function Find-Exe([string]$name, [string[]]$candidates) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    foreach ($p in $candidates) {
        if ($p -and (Test-Path -LiteralPath $p -PathType Leaf)) { return (Resolve-Path -LiteralPath $p).Path }
    }
    return $null
}

$clang = Find-Exe 'clang++.exe' @(
    (Join-Path $env:USERPROFILE 'Desktop\\llvm-mingw-*\\bin\\clang++.exe'),
    (Join-Path $env:USERPROFILE 'Desktop\\ForgeEngine\\llvm-mingw-*\\bin\\clang++.exe'),
    (Join-Path $env:USERPROFILE 'Desktop\\ForgeEngine\\Toolchain\\llvm-mingw-*\\bin\\clang++.exe'),
    (Join-Path $env:ProgramFiles 'LLVM\\bin\\clang++.exe')
)

# Wildcards above may not resolve through Test-Path reliably; enumerate common folders.
if (-not $clang) {
    $roots = @(
        (Join-Path $env:USERPROFILE 'Desktop'),
        (Join-Path $env:USERPROFILE 'Desktop\\ForgeEngine'),
        (Join-Path $env:USERPROFILE 'Desktop\\ForgeEngine\\Toolchain')
    )
    foreach ($root in $roots) {
        if (Test-Path $root) {
            $d = Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -like 'llvm-mingw-*' } |
                Sort-Object LastWriteTime -Descending
            foreach ($dir in $d) {
                $p = Join-Path $dir.FullName 'bin\\clang++.exe'
                if (Test-Path $p) { $clang = (Resolve-Path $p).Path; break }
            }
        }
        if ($clang) { break }
    }
}

if (-not $clang) { throw 'No clang++.exe found. CORE-AI cannot build without an existing C++ compiler.' }

$build = Join-Path (Get-Location) 'build-native'
$pkg = Join-Path (Get-Location) 'package'
Remove-Item -Recurse -Force $build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force $pkg -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $build, (Join-Path $pkg 'bin'), (Join-Path $pkg 'web'), (Join-Path $pkg 'docs') | Out-Null

$sources = @(
 'src/core/RuntimePaths.cpp',
 'src/core/Logger.cpp',
 'src/core/Config.cpp',
 'src/core/Database.cpp',
 'src/core/Capabilities.cpp',
 'src/providers/OllamaProvider.cpp',
 'src/services/CoreApp.cpp',
 'src/services/LocalApiServer.cpp',
 'src/services/LocalFeatures.cpp'
)
$common = @('-std=c++20','-O2','-Iinclude','-Isrc','-DCOREAI_HAS_NATIVE_GUI=1')
$libs = @('-lws2_32','-lwinhttp','-luser32','-lshell32','-ladvapi32')

$appArgs = $common + @('src/main.cpp','src/gui/Win32Gui.cpp') + $sources + $libs + @('-mwindows','-o', (Join-Path $build 'core-ai.exe'))
& $clang @appArgs
if ($LASTEXITCODE -ne 0) { throw 'Native CORE-AI build failed.' }

$testArgs = $common + @('tests/core_tests.cpp') + $sources + $libs + @('-o', (Join-Path $build 'core-ai-tests.exe'))
& $clang @testArgs
if ($LASTEXITCODE -ne 0) { throw 'CORE-AI test binary build failed.' }

& (Join-Path $build 'core-ai-tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'CORE-AI tests failed.' }

Copy-Item (Join-Path $build 'core-ai.exe') (Join-Path $pkg 'bin\\core-ai.exe')
Copy-Item 'web\\*' (Join-Path $pkg 'web') -Recurse -Force
Copy-Item 'docs\\*' (Join-Path $pkg 'docs') -Recurse -Force
Copy-Item 'README.md','BUILD.md','SECURITY.md','CONFIGURATION.md' $pkg -Force

# Copy compiler runtime DLLs when they exist beside clang++.
$toolBin = Split-Path -Parent $clang
foreach ($dll in @('libc++.dll','libc++abi.dll','libwinpthread-1.dll','libgcc_s_seh-1.dll','libunwind.dll')) {
    $src = Join-Path $toolBin $dll
    if (Test-Path $src) { Copy-Item $src (Join-Path $pkg 'bin') -Force }
}

$zip = Join-Path (Get-Location) 'CORE-AI-Windows-x64.zip'
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $pkg '*') -DestinationPath $zip -CompressionLevel Optimal

Write-Host "PASS: compiler=$clang"
Write-Host 'PASS: native build'
Write-Host 'PASS: tests'
Write-Host "PASS: package=$zip"
