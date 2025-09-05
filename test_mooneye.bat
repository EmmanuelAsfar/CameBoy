@echo off
echo ========================================
echo Test des tests Mooneye
echo ========================================

set "test_dir=tests\mooneye\acceptance"
set "output_dir=mooneye_results"

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

echo Test des ROMs Mooneye importantes...
echo.

echo ========================================
echo Test: boot_hwio-dmg0.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\boot_hwio-dmg0.gb" 20000 > "%output_dir%\boot_hwio-dmg0.gb.txt" 2>&1

echo ========================================
echo Test: boot_regs-dmg0.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\boot_regs-dmg0.gb" 20000 > "%output_dir%\boot_regs-dmg0.gb.txt" 2>&1

echo ========================================
echo Test: call_cc_timing.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\call_cc_timing.gb" 20000 > "%output_dir%\call_cc_timing.gb.txt" 2>&1

echo ========================================
echo Test: di_timing-GS.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\di_timing-GS.gb" 20000 > "%output_dir%\di_timing-GS.gb.txt" 2>&1

echo ========================================
echo Test: ei_timing.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\ei_timing.gb" 20000 > "%output_dir%\ei_timing.gb.txt" 2>&1

echo ========================================
echo Test: halt_ime0_ei.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\halt_ime0_ei.gb" 20000 > "%output_dir%\halt_ime0_ei.gb.txt" 2>&1

echo ========================================
echo Test: jp_cc_timing.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\jp_cc_timing.gb" 20000 > "%output_dir%\jp_cc_timing.gb.txt" 2>&1

echo ========================================
echo Test: ret_cc_timing.gb
echo ========================================
.\bin\cameboy_test.exe "tests\mooneye\acceptance\ret_cc_timing.gb" 20000 > "%output_dir%\ret_cc_timing.gb.txt" 2>&1

echo.
echo ========================================
echo Tests Mooneye termines !
echo ========================================
echo Resultats dans le dossier: %output_dir%
echo.
