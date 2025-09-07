@echo off
setlocal

rem Detect project root
for %%I in ("%~dp0..\..") do set "PROJECT_DIR=%%~fI\"

rem Setup GBDK tools
set "LCC=%PROJECT_DIR%tools\gbdk\bin\lcc.exe"

echo [GBDK] Building examples...

rem Change to GBDK directory
cd /d "%~dp0"

rem Build all examples dynamically
for /d %%d in (*) do (
    if exist "%%d\src\main.c" (
        call :build_example "%%d"
    )
)

echo [GBDK] Done.
goto :eof

:build_example
set "EXAMPLE=%~1"
if not exist "%EXAMPLE%\src\main.c" goto :eof

echo Building %EXAMPLE%...
cd /d "%EXAMPLE%"

if not exist "build" mkdir build

"%LCC%" -o build\%EXAMPLE%.gb src/main.c
if errorlevel 1 (
    echo FAILED: %EXAMPLE%
    cd ..
    goto :eof
)

echo OK: %EXAMPLE%
cd ..
goto :eof