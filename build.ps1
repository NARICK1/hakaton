Write-Host "Сборка проекта 'Будни студента'..." -ForegroundColor Green

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vswhere)) {
    Write-Host "vswhere.exe не найден. Visual Studio Installer повреждён или не установлен." -ForegroundColor Red
    exit 1
}

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

if ([string]::IsNullOrWhiteSpace($vsPath)) {
    Write-Host "Visual Studio с C++ не найдена." -ForegroundColor Red
    exit 1
}

$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvars)) {
    Write-Host "vcvars64.bat не найден." -ForegroundColor Red
    exit 1
}

$buildCmd = @"
call "$vcvars"
cl /EHsc /std:c++17 /I. main.cpp core\*.cpp data\*.cpp player\*.cpp npc\*.cpp exams\*.cpp events\*.cpp systems\*.cpp ui\*.cpp save\*.cpp /Fe:student_life.exe
"@

Write-Host "Компиляция..." -ForegroundColor Yellow
cmd /c $buildCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nСборка успешна! Запустите: .\student_life.exe" -ForegroundColor Green
} else {
    Write-Host "`nОшибка сборки. Код: $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}