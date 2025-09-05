@echo off
echo === Compilation de CameBoy (version Win32) === > build_log.txt
echo. >> build_log.txt

REM Créer le dossier bin s'il n'existe pas
if not exist "bin" mkdir bin

echo Compilation... >> build_log.txt
"C:\TDM-GCC-64\bin\gcc.exe" -Wall -Wextra -std=c99 -O2 -g -Isrc src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\graphics_win32.c src\emulator_win32.c -lgdi32 -luser32 -lkernel32 -o bin\cameboy_win32.exe >> build_log.txt 2>&1

if %errorlevel% == 0 (
    echo ✅ Compilation réussie ! >> build_log.txt
    echo. >> build_log.txt
    echo Pour lancer : .\bin\cameboy_win32.exe .\tests\blargg\cpu_instrs\individual\01-special.gb >> build_log.txt
) else (
    echo ❌ Erreur de compilation >> build_log.txt
)

echo Terminé. >> build_log.txt
echo.
echo Voir build_log.txt pour les détails
