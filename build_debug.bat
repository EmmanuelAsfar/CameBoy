@echo off
echo Compiling with debug...
gcc -o bin\cameboy_test.exe src\mmu.c src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\timer.c src\ppu.c src\joypad.c src\graphics_win32.c src\emulator_simple.c -lgdi32 -luser32 2>&1 | tee compile_log.txt
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    type compile_log.txt
    pause
    exit /b 1
)
echo Compilation successful!
echo Running test...
bin\cameboy_test.exe tests\blargg\cpu_instrs\individual\01-special.gb > test_output.txt 2>&1
echo Test completed. Check test_output.txt for results.
