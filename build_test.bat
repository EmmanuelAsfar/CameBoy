@echo off
echo === Compilation de CameBoy pour tests ===
echo.

REM Créer le dossier bin s'il n'existe pas
if not exist "bin" mkdir bin

echo Compilation émulateur simple...
"C:\TDM-GCC-64\bin\gcc.exe" -Wall -Wextra -std=c99 -O2 -g -Isrc src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\emulator_simple.c -o bin\cameboy_test.exe > compilation_log.txt 2>&1

if %errorlevel% neq 0 (
    echo ❌ Erreur de compilation - Voir compilation_log.txt
    type compilation_log.txt
    exit /b 1
)

echo ✅ Compilation réussie !
echo Exécutable : bin\cameboy_test.exe
echo.

echo Test avec la ROM simple...
.\bin\cameboy_test.exe .\tests\test_rom.gb 10000 > test_output.txt 2>&1

echo.
echo Résultat du test dans test_output.txt
type test_output.txt
