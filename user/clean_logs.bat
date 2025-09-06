@echo off
setlocal
set PROJECT_DIR=%~dp0\..
cd /d "%PROJECT_DIR%"
if exist logs\* (
  echo Cleaning logs...
  del /q logs\*.log 2>nul
  del /q logs\*.ppm 2>nul
  if exist logs\blargg\* del /q logs\blargg\* 2>nul
  if exist logs\rom\* del /q logs\rom\* 2>nul
)
echo Done.
pause
