$ErrorActionPreference = 'Stop'

# Stop stale CORE-AI processes so Windows does not keep the previous executable mapped during relink.
Get-Process -Name 'core-ai' -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Set-Location (Join-Path $PSScriptRoot '..')

function Find-Exe([string]$name, [string[]]$roots) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    foreach ($root in $roots) {
        if (-not $root -or -not (Test-Path -LiteralPath $root -PathType Container)) { continue }
        $dirs = Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like 'llvm-mingw-*' } |
            Sort-Object LastWriteTime -Descending
        foreach ($d in $dirs) {
            foreach ($name2 in @('clang++.exe','g++.exe')) {
                $p = Join-Path $d.FullName ("bin\\$name2")
                if (Test-Path -LiteralPath $p -PathType Leaf) { return (Resolve-Path -LiteralPath $p).Path }
            }
        }
    }
    $fixed = @(
        (Join-Path $env:ProgramFiles 'LLVM\\bin\\clang++.exe'),
        (Join-Path ${env:ProgramFiles(x86)} 'LLVM\\bin\\clang++.exe')
    )
    foreach ($p in $fixed) { if ($p -and (Test-Path -LiteralPath $p -PathType Leaf)) { return (Resolve-Path -LiteralPath $p).Path } }
    return $null
}

function Get-ImportedDllNames([string]$exe, [string]$toolBin) {
    $objdumpCandidates = @(
        (Join-Path $toolBin 'llvm-objdump.exe'),
        (Join-Path $toolBin 'objdump.exe')
    ) | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf }
    foreach ($objdump in $objdumpCandidates) {
        $lines = & $objdump '-p' $exe 2>$null
        if ($LASTEXITCODE -eq 0) {
            return @($lines | Where-Object { $_ -match 'DLL Name:' } | ForEach-Object { ($_ -split 'DLL Name:\s*',2)[1].Trim() } | Sort-Object -Unique)
        }
    }
    return @()
}

function Copy-RequiredRuntimeDlls([string]$exe, [string]$toolBin, [string]$destDir) {
    $names = @(Get-ImportedDllNames -exe $exe -toolBin $toolBin)
    $common = @(
        'libstdc++-6.dll',
        'libgcc_s_seh-1.dll',
        'libwinpthread-1.dll',
        'libc++.dll',
        'libc++abi.dll',
        'libunwind.dll'
    )
    foreach ($n in $common) { if ($n -notin $names) { $names += $n } }
    New-Item -ItemType Directory -Force $destDir | Out-Null
    foreach ($n in ($names | Sort-Object -Unique)) {
        # Windows system DLLs are supplied by Windows and must not be copied.
        if ($n -match '^(KERNEL32|USER32|ADVAPI32|SHELL32|WINHTTP|WS2_32|OLE32|OLEAUT32|GDI32|COMDLG32|CRYPT32|RPCRT4|NTDLL)\\.DLL$') { continue }
        $src = Join-Path $toolBin $n
        # Case-insensitive lookup for toolchains with different DLL casing.
        if (-not (Test-Path -LiteralPath $src -PathType Leaf)) {
            $match = Get-ChildItem -LiteralPath $toolBin -File -Filter '*.dll' -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -ieq $n } | Select-Object -First 1
            if ($match) { $src = $match.FullName }
        }
        if (Test-Path -LiteralPath $src -PathType Leaf) {
            Copy-Item -LiteralPath $src -Destination (Join-Path $destDir (Split-Path $src -Leaf)) -Force
        }
    }
}

$clang = Find-Exe 'clang++.exe' @(
    (Join-Path $env:USERPROFILE 'Desktop'),
    (Join-Path $env:USERPROFILE 'Desktop\ForgeEngine'),
    (Join-Path $env:USERPROFILE 'Desktop\ForgeEngine\Toolchain')
)
if (-not $clang) { throw 'No clang++.exe or g++.exe found. This build intentionally does not install a compiler.' }

$toolBin = Split-Path -Parent $clang
$env:PATH = "$toolBin;$env:PATH"

$root = (Get-Location).Path
$build = Join-Path $root 'build-native'
$pkg = Join-Path $root 'package-native'

# Prevent Windows file-lock failures caused by a previous CORE-AI build still running.
$oldExe = Join-Path $build 'core-ai.exe'
if (Test-Path -LiteralPath $oldExe -PathType Leaf) {
    $fullOldExe = [System.IO.Path]::GetFullPath($oldExe)
    Get-Process -Name 'core-ai' -ErrorAction SilentlyContinue | ForEach-Object {
        try {
            $modulePath = $_.MainModule.FileName
            if ($modulePath -and [System.IO.Path]::GetFullPath($modulePath) -ieq $fullOldExe) {
                $_ | Stop-Process -Force -ErrorAction SilentlyContinue
            }
        } catch {}
    }
    Start-Sleep -Milliseconds 200
}
$runtimeDir = Join-Path $build 'runtime'
Remove-Item -Recurse -Force $build,$pkg -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $build,$runtimeDir,(Join-Path $pkg 'bin'),(Join-Path $pkg 'web'),(Join-Path $pkg 'docs') | Out-Null

function Ensure-SqliteDll([string]$destination) {
    $existing = @(
        (Join-Path $root 'vendor\sqlite3.dll'),
        (Join-Path $root 'sqlite3.dll'),
        (Join-Path $env:USERPROFILE 'Desktop\sqlite3.dll')
    ) | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
    if($existing){ Copy-Item -LiteralPath $existing -Destination $destination -Force; return $true }

    $url = 'https://www.sqlite.org/2026/sqlite-dll-win-x64-3530400.zip'
    $download = Join-Path $build 'sqlite-dll.zip'
    $extract = Join-Path $build 'sqlite-extract'
    try {
        Write-Host '[CORE] SQLite DLL not found locally; downloading official SQLite Windows x64 binary.'
        Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $download
        Remove-Item -Recurse -Force $extract -ErrorAction SilentlyContinue
        Expand-Archive -LiteralPath $download -DestinationPath $extract -Force
        $dll = Get-ChildItem -LiteralPath $extract -Recurse -File -Filter 'sqlite3.dll' | Select-Object -First 1
        if(-not $dll){ throw 'sqlite3.dll was not found inside the official SQLite archive.' }
        Copy-Item -LiteralPath $dll.FullName -Destination $destination -Force
        return $true
    } catch {
        throw "SQLite dependency unavailable. Place sqlite3.dll (Windows x64) in vendor\sqlite3.dll or allow the official download: $($_.Exception.Message)"
    }
}

$sources = @(
 'src/core/RuntimePaths.cpp',
 'src/core/UserStorage.cpp',
 'src/core/Logger.cpp',
 'src/core/Config.cpp',
 'src/core/Database.cpp',
 'src/core/Capabilities.cpp',
 'src/providers/OllamaProvider.cpp',
 'src/services/CoreApp.cpp',
 'src/services/LocalApiServer.cpp',
 'src/services/LocalFeatures.cpp'
)
$common = @('-std=c++20','-O2','-Iinclude','-Isrc','-static-libgcc','-static-libstdc++')
$libs = @('-lws2_32','-luser32','-lshell32','-ladvapi32')
$appExe = Join-Path $build 'core-ai.exe'
$testExe = Join-Path $build 'core-ai-tests.exe'

$appArgs = @(); $appArgs += $common; $appArgs += 'src/main.cpp'; $appArgs += 'src/gui/Win32Gui.cpp'; $appArgs += $sources; $appArgs += $libs; $appArgs += '-mconsole'; $appArgs += '-o'; $appArgs += $appExe
& $clang @appArgs
if ($LASTEXITCODE -ne 0) { throw 'Native CORE-AI application build failed.' }

$testArgs = @(); $testArgs += $common; $testArgs += 'tests/core_tests.cpp'; $testArgs += $sources; $testArgs += $libs; $testArgs += '-o'; $testArgs += $testExe
& $clang @testArgs
if ($LASTEXITCODE -ne 0) { throw 'CORE-AI test binary build failed.' }

# SQLite is a required local runtime dependency for CORE-AI. Keep it beside both
# executable and packaged runtime so the app is self-contained on Windows x64.
$sqliteForTest = Join-Path $build 'sqlite3.dll'
Ensure-SqliteDll $sqliteForTest
Copy-Item -LiteralPath $sqliteForTest -Destination (Join-Path $runtimeDir 'sqlite3.dll') -Force
$env:PATH = "$runtimeDir;$build;$toolBin;$env:PATH"

Write-Host '=== CORE-AI LOCAL TESTS ==='
& $testExe
$testExit = $LASTEXITCODE
if ($testExit -ne 0) { throw "CORE-AI tests failed with exit code $testExit." }

Copy-Item $appExe (Join-Path $pkg 'bin\core-ai.exe') -Force
Copy-Item $sqliteForTest (Join-Path $pkg 'bin\sqlite3.dll') -Force
Copy-RequiredRuntimeDlls -exe $appExe -toolBin $toolBin -destDir (Join-Path $pkg 'bin')
if (Test-Path 'web') { Copy-Item 'web\*' (Join-Path $pkg 'web') -Recurse -Force }
if (Test-Path 'docs') { Copy-Item 'docs\*' (Join-Path $pkg 'docs') -Recurse -Force }
foreach ($f in @('README.md','BUILD.md','SECURITY.md','CONFIGURATION.md')) { if (Test-Path $f) { Copy-Item $f $pkg -Force } }

$zip = Join-Path $root 'CORE-AI-Windows-x64.zip'
Remove-Item $zip -Force -ErrorAction SilentlyContinue

# Package only after the test process has fully exited. Windows can briefly keep
# mapped DLL sections open after a native process terminates, so avoid the more
# fragile PowerShell Compress-Archive implementation and prefer the built-in
# Windows tar/bsdtar ZIP writer. Fall back to .NET ZipFile with a short retry.
function Write-ReleaseZip([string]$sourceDir, [string]$destination) {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    for ($attempt = 1; $attempt -le 8; $attempt++) {
        try {
            if (Test-Path $destination) { Remove-Item $destination -Force -ErrorAction SilentlyContinue }
            [System.IO.Compression.ZipFile]::CreateFromDirectory($sourceDir, $destination, [System.IO.Compression.CompressionLevel]::Optimal, $false)
            if (Test-Path -LiteralPath $destination -PathType Leaf) { return }
        } catch {
            if ($attempt -eq 8) { break }
            Start-Sleep -Milliseconds (250 * $attempt)
        }
    }

    $tar = Get-Command 'tar.exe' -ErrorAction SilentlyContinue
    if ($tar) {
        $cwd = Get-Location
        try {
            Set-Location $sourceDir
            & $tar.Source '-a','-cf',$destination,'.'
            if ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath $destination -PathType Leaf)) { return }
        } finally {
            Set-Location $cwd
        }
    }
    throw 'Release ZIP could not be created.'
}

Start-Sleep -Milliseconds 500
Write-ReleaseZip -sourceDir $pkg -destination $zip

# Verify the archive can be opened and has the runnable entry point before PASS.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::OpenRead($zip)
try {
    $entry = $archive.Entries | Where-Object { ($_.FullName.TrimStart('./').Replace('\','/')) -ieq 'bin/core-ai.exe' } | Select-Object -First 1
    if (-not $entry) { throw 'Release archive is missing bin/core-ai.exe.' }
} finally {
    $archive.Dispose()
}

Write-Host "PASS: compiler=$clang"
Write-Host 'PASS: native build'
Write-Host 'PASS: SQLite runtime staged'
Write-Host 'PASS: local tests'
Write-Host 'PASS: package created'
Write-Host 'PASS: package archive verified'
Write-Host "Package: $zip"
