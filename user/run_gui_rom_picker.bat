@echo off
setlocal
set PROJECT_DIR=%~dp0\..
cd /d "%PROJECT_DIR%"
if "%~1"=="" (
  echo Drag and drop a .gb file onto this .bat, or pass a path.
  pause
  exit /b 1
)
call cameboy.bat build >nul 2>&1
call cameboy.bat run "%~1"
pause
