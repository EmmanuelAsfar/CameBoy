@echo off
setlocal

rem Detect project root
for %%I in ("%~dp0..\..") do set "PROJECT_DIR=%%~fI\"

rem Setup RGBDS tools
set "RGBASM=%PROJECT_DIR%tools\rgbds\bin\rgbasm.exe"
set "RGBLINK=%PROJECT_DIR%tools\rgbds\bin\rgblink.exe"
set "RGBFIX=%PROJECT_DIR%tools\rgbds\bin\rgbfix.exe"

echo [RGBDS] Building examples...

rem Change to RGBDS directory
cd /d "%~dp0"

rem Build all examples dynamically
for /d %%d in (*) do (
    if exist "%%d\src\main.asm" (
        call :build_example "%%d"
    )
)

echo [RGBDS] Done.
goto :eof

:build_example
set "EXAMPLE=%~1"
if not exist "%EXAMPLE%\src\main.asm" goto :eof

echo Building %EXAMPLE%...
cd /d "%EXAMPLE%"

if not exist "build" mkdir build

"%RGBASM%" -o build\%EXAMPLE%.o src/main.asm
if errorlevel 1 (
    echo FAILED: %EXAMPLE%
    cd ..
    goto :eof
)

"%RGBLINK%" -o build\%EXAMPLE%.gb build\%EXAMPLE%.o
if errorlevel 1 (
    echo FAILED: %EXAMPLE%
    cd ..
    goto :eof
)

"%RGBFIX%" -v -p 0xFF build\%EXAMPLE%.gb
if errorlevel 1 (
    echo FAILED: %EXAMPLE%
    cd ..
    goto :eof
)

echo OK: %EXAMPLE%
cd ..
goto :eof