$ErrorActionPreference='Stop'
Set-Location $PSScriptRoot\..
$build='build-windows'
cmake -S . -B $build -A x64 -DCOREAI_BUILD_TESTS=ON -DCOREAI_BUILD_NATIVE_GUI=ON
if($LASTEXITCODE -ne 0){throw 'CMake configure failed'}
cmake --build $build --config Release
if($LASTEXITCODE -ne 0){throw 'Build failed'}
ctest --test-dir $build -C Release --output-on-failure
if($LASTEXITCODE -ne 0){throw 'Tests failed'}
cmake --install $build --config Release --prefix package
if($LASTEXITCODE -ne 0){throw 'Install failed'}
Write-Host 'CORE-AI Windows build/tests/install PASS'
