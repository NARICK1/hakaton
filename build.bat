@echo off
chcp 65001 >nul
echo Сборка проекта "Будни студента"...
echo.
REM ---- Поиск Visual Studio 2022 ----
set "VS_PATH="

IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    goto :found
)

IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    goto :found
)

IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    goto :found
)

IF EXIST "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
    goto :found
)

echo Не удалось найти Visual Studio 2022.
pause
exit /b 1

:found
echo Найдена VS: %VS_PATH%
call "%VS_PATH%"
if %errorlevel% neq 0 (
    echo Ошибка подключения среды Visual Studio.
    pause
    exit /b 1
)

REM ---- Сборка ----
echo.
echo Компиляция...

cl /EHsc /std:c++17 /I. /Fe:student_life.exe ^
    main.cpp ^
    core\Game.cpp ^
    core\GameState.cpp ^
    player\Player.cpp ^
    npc\NPC.cpp ^
    data\AsciiArt.cpp ^
    data\Lang.cpp ^
    exams\Exam.cpp ^
    events\RandomEvent.cpp ^
    systems\Achievements.cpp ^
    systems\DebuffSystem.cpp ^
    systems\Encyclopedia.cpp ^
    systems\EndingSystem.cpp ^
    systems\EventJournal.cpp ^
    systems\FatigueSystem.cpp ^
    systems\Habits.cpp ^
    systems\HungerSystem.cpp ^
    systems\RelationshipSystem.cpp ^
    systems\ReputationSystem.cpp ^
    ui\ConsoleUI.cpp ^
    ui\Credits.cpp ^
    ui\DevMode.cpp ^
    ui\Menu.cpp ^
    save\SaveManager.cpp

if %errorlevel% equ 0 (
    echo.
    echo Сборка успешна! Запустите student_life.exe
) else (
    echo.
    echo Ошибка сборки.
)

pause
