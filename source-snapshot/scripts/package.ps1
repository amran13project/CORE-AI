$ErrorActionPreference='Stop'
Set-Location (Join-Path $PSScriptRoot '..')
if($env:OS -ne 'Windows_NT'){ throw 'Windows package creation must run on Windows; release verification is NOT_VERIFIED here.' }
$version='0.3.0'
$stage='package'
$zip="CORE-AI-$version-Windows-x64.zip"
if(Test-Path $stage){Remove-Item $stage -Recurse -Force}
New-Item -ItemType Directory $stage | Out-Null
if(-not (Test-Path 'build-windows\Release\core-ai.exe')){ throw 'Verified Windows build executable not found.' }
Copy-Item 'build-windows\Release\core-ai.exe' $stage
Copy-Item -Recurse web $stage
Copy-Item -Recurse docs $stage
Copy-Item README.md,BUILD.md,SECURITY.md,CONFIGURATION.md $stage
Compress-Archive -Path "$stage\*" -DestinationPath $zip -Force
Write-Host $zip
