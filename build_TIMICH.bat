@echo off
chcp 65001 >nul

echo Сборка проекта "Будни студента"...
echo.

set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist %VSWHERE% (
    echo vswhere.exe не найден. Visual Studio Installer повреждён или не установлен.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_PATH=%%i
)

if "%VS_PATH%"=="" (
    echo Visual Studio с C++ не найдена.
    pause
    exit /b 1
)

call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

if %errorlevel% neq 0 (
    echo Не удалось инициализировать окружение Visual Studio.
    pause
    exit /b 1
)

echo.
echo Компиляция...

cl /EHsc /std:c++17 /I. ^
main.cpp ^
core\*.cpp ^
data\*.cpp ^
player\*.cpp ^
npc\*.cpp ^
exams\*.cpp ^
events\*.cpp ^
systems\*.cpp ^
ui\*.cpp ^
save\*.cpp ^
/Fe:student_life.exe

if %errorlevel% equ 0 (
    echo.
    echo Сборка успешна! Запустите student_life.exe
) else (
    echo.
    echo Ошибка сборки.
)

pause