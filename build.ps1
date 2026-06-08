param()

Write-Host "Сборка проекта 'Будни студента'..." -ForegroundColor Green

$vcvars = "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvars)) {
    Write-Host "Visual Studio не найдена." -ForegroundColor Red
    exit 1
}

$sourceFiles = @(
    "main.cpp",
    "core\Game.cpp", "core\GameState.cpp",
    "player\Player.cpp",
    "npc\NPC.cpp",
    "exams\Exam.cpp",
    "events\RandomEvent.cpp",
    "systems\RelationshipSystem.cpp", "systems\FatigueSystem.cpp",
    "systems\HungerSystem.cpp", "systems\ReputationSystem.cpp", "systems\EndingSystem.cpp",
    "ui\ConsoleUI.cpp", "ui\Menu.cpp",
    "save\SaveManager.cpp"
)

$sources = $sourceFiles -join " "
$buildCmd = "`"$vcvars`" >nul && cl /EHsc /std:c++17 /I. /Fe:student_life.exe $sources"

Write-Host "Компиляция..." -ForegroundColor Yellow
cmd /c $buildCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nСборка успешна! Запустите: .\student_life.exe" -ForegroundColor Green
} else {
    Write-Host "`nОшибка сборки (код: $LASTEXITCODE)" -ForegroundColor Red
}
