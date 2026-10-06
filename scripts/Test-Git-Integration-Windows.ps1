$ErrorActionPreference='Stop'
Set-Location (Join-Path $PSScriptRoot '..')
$git=Get-Command git.exe -ErrorAction SilentlyContinue
if(-not $git){Write-Host 'SKIPPED — Git executable unavailable'; exit 0}
$root=Join-Path $env:TEMP 'core-ai-git-integration-test'
Remove-Item -Recurse -Force $root -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $root | Out-Null
Push-Location $root
try {
  & $git.Source init -q
  if($LASTEXITCODE -ne 0){throw 'git init failed'}
  Set-Content -Path README.md -Value 'CORE-AI Git integration test'
  $out=& $git.Source status --short 2>&1
  if($LASTEXITCODE -ne 0){throw 'git status failed'}
  if(-not ($out -match 'README.md')){throw 'expected README.md in Git status'}
  Write-Host 'PASS — Git init/status integration'
} finally { Pop-Location; Remove-Item -Recurse -Force $root -ErrorAction SilentlyContinue }
