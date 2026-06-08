@echo off
chcp 65001 >nul
title Сборка "Будни студента"
setlocal enabledelayedexpansion

echo === Сборка проекта "Будни студента" ===
echo.

set "VS_PATH="

REM ===== 1. Поиск через vswhere.exe =====
set "vsw=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vsw%" set "vsw=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%vsw%" (
    for /f "usebackq tokens=*" %%i in (`"%vsw%" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -latest 2^>nul`) do (
        if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" set "VS_PATH=%%i\VC\Auxiliary\Build\vcvars64.bat"
    )
    if defined VS_PATH goto :found
)

REM ===== 2. Fallback: известные пути (год + редакция) =====
for %%p in (
    "%ProgramFiles%\Microsoft Visual Studio\2022\Community"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"
    "%ProgramFiles%\Microsoft Visual Studio\2019\Community"
    "%ProgramFiles%\Microsoft Visual Studio\2019\Professional"
    "%ProgramFiles%\Microsoft Visual Studio\2019\Enterprise"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools"
) do if exist "%%~p\VC\Auxiliary\Build\vcvars64.bat" set "VS_PATH=%%~p\VC\Auxiliary\Build\vcvars64.bat" & goto :found

REM ===== 3. Fallback: Insider / Preview (версионный путь) =====
for %%v in (22 21 20 19 18 17 16) do for %%e in (Insiders Preview Community Professional Enterprise BuildTools) do (
    if exist "%ProgramFiles%\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VS_PATH=%ProgramFiles%\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" & goto :found
)

REM ===== 4. Fallback: vswhere в PATH (для совместимости) =====
where vswhere >nul 2>nul
if !errorlevel! equ 0 (
    for /f "usebackq tokens=*" %%i in (`vswhere -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -latest 2^>nul`) do (
        if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" set "VS_PATH=%%i\VC\Auxiliary\Build\vcvars64.bat"
    )
    if defined VS_PATH goto :found
)

REM ===== Не найдено =====
echo.
echo [ОШИБКА] Visual Studio C++ не найдена.
echo.
echo Для сборки проекта требуется установить:
echo   - Visual Studio 2022/2019 ^(Community/Professional/Enterprise^)
echo     или
echo   - Visual Studio Build Tools ^(отдельный установщик^)
echo.
echo Скачать Build Tools:
echo   https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
echo.
echo ИЛИ скачайте готовую сборку из раздела Releases на GitHub.
echo.
pause
exit /b 1

REM ===== VS найдена =====
:found
echo [OK] Найдена VS: %VS_PATH%
echo.
call "%VS_PATH%"
if %errorlevel% neq 0 (
    echo.
    echo [ОШИБКА] Не удалось загрузить окружение Visual Studio.
    pause
    exit /b 1
)

REM ===== Сборка =====
echo Компиляция...
echo.

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
    echo === Сборка успешна! ===
    echo Запустите: student_life.exe
) else (
    echo.
    echo [ОШИБКА] Сборка не удалась.
)

echo.
pause
