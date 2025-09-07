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
if exist "%LOCAL_RGBDS%\rgbasm.exe" (
  rem ok
) else if defined RGBDS_HOME (
  rem ok
) else (
  rem If relying on PATH, skip check
)

set "OUTDIR=%~dp0build"
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo [RGBDS] Assembling...
"%RGBASM%" -o "%OUTDIR%\main.o" "%~dp0src\main.asm" || goto :error

echo [RGBDS] Linking...
"%RGBLINK%" -o "%OUTDIR%\rgbds_hello.gb" "%OUTDIR%\main.o" || goto :error

echo [RGBDS] Fixing header...
"%RGBFIX%" -v -p 0 -t "RGBHELLO" "%OUTDIR%\rgbds_hello.gb" || goto :error

echo Success: "%OUTDIR%\rgbds_hello.gb"
exit /b 0

:error
echo Build failed (RGBDS). >&2
exit /b 1


