$ErrorActionPreference='Stop'
Set-Location (Join-Path $PSScriptRoot '..')
if($env:OS -ne 'Windows_NT'){ throw 'Windows package creation must run on Windows; release verification is NOT_VERIFIED here.' }
$version='0.3.2'
$stage='package'
$zip="CORE-AI-$version-Windows-x64.zip"
if(Test-Path $stage){Remove-Item $stage -Recurse -Force}
New-Item -ItemType Directory $stage | Out-Null
if(-not (Test-Path 'build-windows\Release\core-ai.exe')){ throw 'Verified Windows build executable not found.' }
Copy-Item 'build-windows\Release\core-ai.exe' $stage
Copy-Item -Recurse web $stage
Copy-Item -Recurse docs $stage
Copy-Item README.md,BUILD.md,SECURITY.md,CONFIGURATION.md $stage
Add-Type -AssemblyName System.IO.Compression.FileSystem
$tar = Get-Command 'tar.exe' -ErrorAction SilentlyContinue
if ($tar) {
  Push-Location $stage
  try { & $tar.Source '-a','-cf',(Join-Path (Split-Path $PWD -Parent) $zip),'.'; if ($LASTEXITCODE -ne 0) { throw 'tar ZIP creation failed.' } } finally { Pop-Location }
} else { [System.IO.Compression.ZipFile]::CreateFromDirectory((Resolve-Path $stage).Path,$zip,[System.IO.Compression.CompressionLevel]::Optimal,$false) }
$archive=[System.IO.Compression.ZipFile]::OpenRead((Resolve-Path $zip).Path); try { if(-not ($archive.Entries.FullName -contains 'core-ai.exe')){throw 'Package missing core-ai.exe.'} } finally {$archive.Dispose()}
Write-Host $zip
