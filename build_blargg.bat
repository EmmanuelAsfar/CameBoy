@echo off
echo === Compilation de CameBoy (version Blargg) === > build_blargg_log.txt
echo. >> build_blargg_log.txt

REM Créer le dossier bin s'il n'existe pas
if not exist "bin" mkdir bin

echo Compilation... >> build_blargg_log.txt
"C:\TDM-GCC-64\bin\gcc.exe" -Wall -Wextra -std=c99 -O2 -g -Isrc src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\emulator_blargg.c -o bin\cameboy_blargg.exe >> build_blargg_log.txt 2>&1

if %errorlevel% == 0 (
    echo ✅ Compilation réussie ! >> build_blargg_log.txt
    echo. >> build_blargg_log.txt
    echo Pour lancer : .\bin\cameboy_blargg.exe .\tests\blargg\cpu_instrs\individual\01-special.gb >> build_blargg_log.txt
) else (
    echo ❌ Erreur de compilation >> build_blargg_log.txt
)

echo Terminé. >> build_blargg_log.txt
echo.
echo Voir build_blargg_log.txt pour les détails
