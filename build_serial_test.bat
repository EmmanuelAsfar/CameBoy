@echo off
echo === Compilation du test serial ===
echo.

echo Compilation test serial...
gcc -Wall -Wextra -std=c99 -O2 -g -Isrc test_serial.c src\mmu.c src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\timer.c src\ppu.c src\joypad.c -o test_serial.exe > serial_compilation_log.txt 2>&1

if %errorlevel% neq 0 (
    echo ❌ Erreur de compilation - Voir serial_compilation_log.txt
    type serial_compilation_log.txt
    exit /b 1
)

echo ✅ Compilation réussie !
echo Exécutable : test_serial.exe
echo.

echo Test du port série...
.\test_serial.exe > serial_test_output.txt 2>&1

echo.
echo Résultat du test dans serial_test_output.txt
type serial_test_output.txt
