@echo off
setlocal
set PROJECT_DIR=%~dp0\..
cd /d "%PROJECT_DIR%"
call cameboy.bat build >nul 2>&1
call cameboy.bat run
pause
