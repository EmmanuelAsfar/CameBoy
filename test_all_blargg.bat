@echo off
echo ========================================
echo Test de tous les tests Blargg CPU
echo ========================================

set "test_dir=tests\blargg\cpu_instrs\individual"
set "output_dir=blargg_results"

if not exist "%output_dir%" mkdir "%output_dir%"

echo.
echo Compilation de l'emulateur...
"C:\TDM-GCC-64\bin\gcc.exe" -Wall -Wextra -std=c99 -O2 -g -Isrc src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\emulator_simple.c -o bin\cameboy_test.exe > compilation_log.txt 2>&1

if %errorlevel% neq 0 (
    echo ERREUR: Compilation echouee
    type compilation_log.txt
    exit /b 1
)

echo Compilation reussie !
echo.

echo Test des ROMs individuelles CPU...
echo.

for %%f in ("%test_dir%\*.gb") do (
    echo ========================================
    echo Test: %%~nxf
    echo ========================================
    
    echo Execution avec 10000 cycles...
    .\bin\cameboy_test.exe "%%f" 10000 > "%output_dir%\%%~nxf.txt" 2>&1
    
    echo Resultat dans: "%output_dir%\%%~nxf.txt"
    echo.
)

echo.
echo ========================================
echo Test du ROM principal CPU
echo ========================================
echo Test: cpu_instrs.gb
echo Execution avec 50000 cycles...
.\bin\cameboy_test.exe "tests\blargg\cpu_instrs\cpu_instrs.gb" 50000 > "%output_dir%\cpu_instrs.gb.txt" 2>&1

echo.
echo ========================================
echo Tests termines !
echo ========================================
echo Resultats dans le dossier: %output_dir%
echo.
