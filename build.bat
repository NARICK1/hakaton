@echo off
chcp 65001 >nul

echo Сборка проекта "Будни студента"...
echo.

set VS2022="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set VS2022_BUILDTOOLS="C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set VS2022_PREVIEW="C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat"
set VS2022_INSIDERS="C:\Program Files\Microsoft Visual Studio\2022\Insiders\VC\Auxiliary\Build\vcvars64.bat"

if exist %VS2022% (
    call %VS2022%
) else if exist %VS2022_BUILDTOOLS% (
    call %VS2022_BUILDTOOLS%
) else if exist %VS2022_PREVIEW% (
    call %VS2022_PREVIEW%
) else if exist %VS2022_INSIDERS% (
    call %VS2022_INSIDERS%
) else (
    echo VS2022 не найден, ищу другую версию...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
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