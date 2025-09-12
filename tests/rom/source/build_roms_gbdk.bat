@echo off
REM Build GBDK-based test ROMs from tests\rom\source\gbdk\<component>\

setlocal enabledelayedexpansion
chcp 65001 >nul

set "ROOT=%~dp0..\..\.."
set "SRC_DIR=%~dp0gbdk"
set "OUT_DIR=%ROOT%\tests\rom"
set "GBDK_DIR=%ROOT%\tools\gbdk"
set "GBDK_BIN=%GBDK_DIR%\bin\lcc.exe"
set "GBDK_LOG=%ROOT%\logs\rom_gbdk_build.log"

if not exist "%GBDK_BIN%" (
  echo [GBDK] Outils introuvables: %GBDK_BIN%
  exit /b 0
)

if not exist "%SRC_DIR%" (
  echo [GBDK] Aucun dossier source: %SRC_DIR%
  exit /b 0
)

echo ========================================
echo [GBDK] Build des ROMs de test
echo Source: %SRC_DIR%
echo Sortie: %OUT_DIR%
echo Log: %GBDK_LOG%
echo ========================================

set "OLDPATH=%PATH%"
set "PATH=%GBDK_DIR%\bin;%PATH%"
set "GBDK_HOME=%GBDK_DIR%"
if not exist "%ROOT%\logs" mkdir "%ROOT%\logs" 2>nul
echo [GBDK] Build started %DATE% %TIME% > "%GBDK_LOG%"

for /d %%D in ("%SRC_DIR%\*") do (
  if exist "%%D\main.c" (
    set "NAME=%%~nD"
    echo [GBDK] Compilation %%~nD
    echo --- %%~nD --- >> "%GBDK_LOG%"
    REM Use short path to avoid spaces breaking SDCC args
    for %%P in ("%%D\main.c") do set "SRC_SHORT=%%~sP"
    "%GBDK_BIN%" -o "%%~nD.gb" "!SRC_SHORT!" 1>> "%GBDK_LOG%" 2>>&1
    if errorlevel 1 (
      echo [GBDK] ERREUR build %%~nD
      echo [GBDK] ERREUR build %%~nD >> "%GBDK_LOG%"
    ) else (
      move /Y "%%~nD.gb" "%OUT_DIR%\%%~nD.gb" >nul
      echo [GBDK] OK %%~nD ^> tests\rom\%%~nD.gb
      echo [GBDK] OK %%~nD >> "%GBDK_LOG%"
    )
  )
)

set "PATH=%OLDPATH%"
exit /b 0
