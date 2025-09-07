@echo off
setlocal ENABLEDELAYEDEXPANSION

rem Detect project root (normalize to absolute path with trailing backslash)
for %%I in ("%~dp0..\..") do set "PROJECT_DIR=%%~fI\"

rem Prefer repo-local tools over env/PATH
set "LOCAL_RGBDS=%PROJECT_DIR%tools\rgbds\bin"
if exist "%LOCAL_RGBDS%\rgbasm.exe" (
  set "RGBASM=%LOCAL_RGBDS%\rgbasm.exe"
  set "RGBLINK=%LOCAL_RGBDS%\rgblink.exe"
  set "RGBFIX=%LOCAL_RGBDS%\rgbfix.exe"
) else if defined RGBDS_HOME (
  set "RGBASM=%RGBDS_HOME%\bin\rgbasm.exe"
  set "RGBLINK=%RGBDS_HOME%\bin\rgblink.exe"
  set "RGBFIX=%RGBDS_HOME%\bin\rgbfix.exe"
) else (
  set "RGBASM=rgbasm"
  set "RGBLINK=rgblink"
  set "RGBFIX=rgbfix"
)

echo [RGBDS] Tool rgbasm: "%RGBASM%"
echo [RGBDS] Local RGBDS bin: "%LOCAL_RGBDS%"

REM Change to script directory
pushd "%~dp0"

REM Create build directory
if not exist "build" mkdir build

REM Assemble
echo [RGBDS] Assembling joypad...
"%RGBASM%" -o build/joypad.o src/main.asm
if errorlevel 1 (
    echo Build failed (RGBDS joypad).
    popd
    exit /b 1
)

REM Link
echo [RGBDS] Linking joypad...
"%RGBLINK%" -o build/rgbds_joypad.gb build/joypad.o
if errorlevel 1 (
    echo Build failed (RGBDS joypad link).
    popd
    exit /b 1
)

REM Fix header
echo [RGBDS] Fixing header joypad...
"%RGBFIX%" -v -p 0xFF build/rgbds_joypad.gb
if errorlevel 1 (
    echo Build failed (RGBDS joypad fix).
    popd
    exit /b 1
)

echo Success: "%CD%\build\rgbds_joypad.gb"
popd
