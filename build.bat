@echo off
echo Compilation de CameBoy avec TDM-GCC-64...

& "C:\TDM-GCC-64\bin\gcc.exe" -Wall -Wextra -std=c99 -O2 -g -Isrc src\*.c -o bin\cameboy.exe

if %errorlevel% == 0 (
    echo ✅ Compilation réussie !
    echo Exécutable : bin\cameboy.exe
) else (
    echo ❌ Erreur de compilation
)

pause