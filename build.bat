@echo off
chcp 65001 >nul
echo Сборка проекта "Будни студента"...
echo.

call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat" 2>nul || (
    echo VS2022 не найден, ищу другую версию...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul || (
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" 2>nul || (
            call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul || (
                echo Не удалось найти Visual Studio.
                pause
                exit /b 1
            )
        )
    )
)

cl /EHsc /std:c++17 /I. /Fe:student_life.exe main.cpp core\*.cpp player\*.cpp npc\*.cpp exams\*.cpp events\*.cpp systems\*.cpp ui\*.cpp save\*.cpp

if %errorlevel% equ 0 (
    echo.
    echo Сборка успешна! Запустите student_life.exe
) else (
    echo.
    echo Ошибка сборки.
)

pause
