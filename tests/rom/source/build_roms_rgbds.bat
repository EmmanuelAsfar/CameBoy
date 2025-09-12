@echo off
REM Build RGBDS-based test ROMs from tests\rom\source\rgbds\<component>\

setlocal enabledelayedexpansion
chcp 65001 >nul

set "ROOT=%~dp0..\..\.."
set "SRC_DIR=%~dp0rgbds"
set "OUT_DIR=%ROOT%\tests\rom"
set "RGBASM=%ROOT%\tools\rgbds\bin\rgbasm.exe"
set "RGBLINK=%ROOT%\tools\rgbds\bin\rgblink.exe"
set "RGBFIX=%ROOT%\tools\rgbds\bin\rgbfix.exe"

if not exist "%RGBASM%" (
  echo [RGBDS] Outils introuvables: %RGBASM%
  exit /b 0
)
if not exist "%RGBLINK%" (
  echo [RGBDS] Outils introuvables: %RGBLINK%
  exit /b 0
)
if not exist "%RGBFIX%" (
  echo [RGBDS] Outils introuvables: %RGBFIX%
  exit /b 0
)

if not exist "%SRC_DIR%" (
  echo [RGBDS] Aucun dossier source: %SRC_DIR%
  exit /b 0
)

echo ========================================
echo [RGBDS] Build des ROMs de test
echo Source: %SRC_DIR%
echo Sortie: %OUT_DIR%
echo ========================================

for /d %%D in ("%SRC_DIR%\*") do (
  if exist "%%D\main.asm" (
    set "NAME=%%~nD"
    echo [RGBDS] Compilation %%~nD
    "%RGBASM%" -o "%%~nD.o" "%%D\main.asm" >nul 2>&1
    if errorlevel 1 (
      echo [RGBDS] ERREUR rgbasm %%~nD
      del /q "%%~nD.o" 2>nul
    ) else (
      "%RGBLINK%" -o "%%~nD.gb" "%%~nD.o" >nul 2>&1
      if errorlevel 1 (
        echo [RGBDS] ERREUR rgblink %%~nD
        del /q "%%~nD.o" 2>nul
        del /q "%%~nD.gb" 2>nul
      ) else (
        "%RGBFIX%" -v -p 0 "%%~nD.gb" >nul 2>&1
        if errorlevel 1 (
          echo [RGBDS] ERREUR rgbfix %%~nD
          del /q "%%~nD.o" 2>nul
          del /q "%%~nD.gb" 2>nul
        ) else (
          move /Y "%%~nD.gb" "%OUT_DIR%\%%~nD.gb" >nul
          del /q "%%~nD.o" 2>nul
          echo [RGBDS] OK %%~nD ^> tests\rom\%%~nD.gb
        )
      )
    )
  )
)

exit /b 0

