@echo off
echo === Compilation de CameBoy avec CPU complet ===
echo.

echo Compilation...
"C:\TDM-GCC-64\bin\gcc.exe" -Wall -Wextra -std=c99 -O2 -g -Isrc src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\graphics_win32.c src\emulator_win32.c -o bin\cameboy_complete.exe -lgdi32 -luser32 -lkernel32

if %errorlevel% neq 0 (
    echo ❌ Erreur de compilation
    pause
    exit /b 1
)

echo ✅ Compilation réussie !
echo Exécutable : bin\cameboy_complete.exe
echo.

echo Test avec la première ROM Blargg...
.\bin\cameboy_complete.exe .\tests\blargg\cpu_instrs\individual\01-special.gb

pause
