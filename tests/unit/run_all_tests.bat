@echo off
REM Script pour lancer tous les tests unitaires sous Windows
REM À exécuter depuis la racine du projet

echo === LANCEMENT DE TOUS LES TESTS UNITAIRES ===
echo.

REM Compiler et lancer les tests CPU
echo Compilation test_cpu.c...
gcc -I src -o tests/unit/test_cpu tests/unit/test_cpu.c src/cpu.c src/cpu_tables.c src/mmu.c src/common.c
if %errorlevel% equ 0 (
    echo Lancement test CPU...
    tests\unit\test_cpu.exe
    echo.
) else (
    echo ❌ Erreur compilation CPU
)

REM Compiler et lancer les tests MMU
echo Compilation test_mmu.c...
gcc -I src -o tests/unit/test_mmu tests/unit/test_mmu.c src/mmu.c src/common.c
if %errorlevel% equ 0 (
    echo Lancement test MMU...
    tests\unit\test_mmu.exe
    echo.
) else (
    echo ❌ Erreur compilation MMU
)

REM Compiler et lancer les tests PPU
echo Compilation test_ppu.c...
gcc -I src -o tests/unit/test_ppu tests/unit/test_ppu.c src/ppu.c src/common.c
if %errorlevel% equ 0 (
    echo Lancement test PPU...
    tests\unit\test_ppu.exe
    echo.
) else (
    echo ❌ Erreur compilation PPU
)

REM Compiler et lancer les tests Timer
echo Compilation test_timer.c...
gcc -I src -o tests/unit/test_timer tests/unit/test_timer.c src/timer.c src/common.c
if %errorlevel% equ 0 (
    echo Lancement test Timer...
    tests\unit\test_timer.exe
    echo.
) else (
    echo ❌ Erreur compilation Timer
)

REM Compiler et lancer les tests Interrupt
echo Compilation test_interrupt.c...
gcc -I src -o tests/unit/test_interrupt tests/unit/test_interrupt.c src/interrupt.c src/cpu.c src/cpu_tables.c src/mmu.c src/common.c
if %errorlevel% equ 0 (
    echo Lancement test Interruptions...
    tests\unit\test_interrupt.exe
    echo.
) else (
    echo ❌ Erreur compilation Interruptions
)

REM Compiler et lancer les tests Joypad
echo Compilation test_joypad.c...
gcc -I src -o tests/unit/test_joypad tests/unit/test_joypad.c src/joypad.c src/common.c
if %errorlevel% equ 0 (
    echo Lancement test Joypad...
    tests\unit\test_joypad.exe
    echo.
) else (
    echo ❌ Erreur compilation Joypad
)

echo === FIN DES TESTS UNITAIRES ===
pause
