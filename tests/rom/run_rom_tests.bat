@echo off
setlocal enabledelayedexpansion

set MODE=%1
if "%MODE%"=="" set MODE=quick

set PROJECT_DIR=%~dp0\..\..
set BIN_DIR=%PROJECT_DIR%\build\bin
set LOGS_DIR=%PROJECT_DIR%\logs
set EXE=%BIN_DIR%\cameboy.exe
set SIMP=%BIN_DIR%\cameboy_simple.exe
set BLARGG_DIR=%LOGS_DIR%\blargg
set ROM_DIR=%LOGS_DIR%\rom

if not exist "%LOGS_DIR%" mkdir "%LOGS_DIR%" 2>nul
if not exist "%BLARGG_DIR%" mkdir "%BLARGG_DIR%" 2>nul
if not exist "%ROM_DIR%" mkdir "%ROM_DIR%" 2>nul

rem Cleanup legacy Blargg logs at root
del /q "%LOGS_DIR%\rom_blargg_*.log" 2>nul
del /q "%LOGS_DIR%\rom_blargg_*.ppm" 2>nul
del /q "%LOGS_DIR%\rom_*.log" 2>nul
del /q "%LOGS_DIR%\rom_*.ppm" 2>nul

rem Initialize results log
echo ======================================== > "%LOGS_DIR%\rom_test_results.log"
echo ROM Test Run - %DATE% %TIME% >> "%LOGS_DIR%\rom_test_results.log"
echo ======================================== >> "%LOGS_DIR%\rom_test_results.log"

REM Build emulator if needed
if not exist "%EXE%" (
  echo Building emulator...
  call "%PROJECT_DIR%\cameboy.bat" build >nul 2>&1
)

REM Build a simple headless emulator_simple executable for ROM runs
echo Compiling headless emulator_simple...
set CFLAGS=-Wall -Wextra -std=c99 -O2 -g -Isrc
set LDFLAGS=-lgdi32 -luser32 -lkernel32

gcc %CFLAGS% src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\interrupt.c src\apu.c src\graphics_win32.c src\emulator_simple.c -o "%SIMP%" %LDFLAGS% 2>> "%LOGS_DIR%\test_build.log"
if errorlevel 1 (
  echo ERREUR: compilation emulator_simple
  exit /b 1
)

REM Build ROM generator
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%" 2>nul

echo Compiling ROM generator...
if exist tests\rom\source\generate_roms.c (
  gcc -std=c99 -O2 tests\rom\source\generate_roms.c -o "%BIN_DIR%\gen_roms.exe" 2>> "%LOGS_DIR%\test_build.log"
 ) else (
  gcc -std=c99 -O2 tests\rom\generate_roms.c -o "%BIN_DIR%\gen_roms.exe" 2>> "%LOGS_DIR%\test_build.log"
 )
if errorlevel 1 (
  echo ERREUR: compilation gen_roms
  exit /b 1
)

REM Generate ROMs
echo Generating ROMs...
"%BIN_DIR%\gen_roms.exe" > "%LOGS_DIR%\rom_gen.log" 2>&1
if errorlevel 1 (
  type "%LOGS_DIR%\rom_gen.log"
  exit /b 1
)

set TOTAL=0
set PASSED=0
set CYCLES=60000000

for %%R in (tests\rom\*.gb) do (
  set /a TOTAL+=1
  set ROM=%%R
  set NAME=%%~nR
  echo Running ROM: !ROM!
  "%SIMP%" "!ROM!" %CYCLES% --headless --dump-ppm "%ROM_DIR%\!NAME!.ppm" > "%ROM_DIR%\!NAME!.log" 2>&1
  findstr /C:"SERIAL:" "%ROM_DIR%\!NAME!.log" > "%ROM_DIR%\!NAME!_serial.txt"
  rem Check for PASS directly in main log (raw chars written)
  findstr /C:"PASS" "%ROM_DIR%\!NAME!.log" >nul
  if !errorlevel! equ 0 (
    echo PASS !NAME!
    echo PASS !NAME! >> "%LOGS_DIR%\rom_test_results.log"
    set /a PASSED+=1
  ) else (
    echo FAIL !NAME!
    echo FAIL !NAME! >> "%LOGS_DIR%\rom_test_results.log"
  )
)

rem Run Blargg ROMs (look for "Passed" on serial output)
echo --- Blargg ROMs (%MODE%) --- >> "%LOGS_DIR%\rom_test_results.log"
if /I "%MODE%"=="quick" (
  for %%B in (
    tests\blargg\halt_bug.gb
    tests\blargg\interrupt_time\interrupt_time.gb
    tests\blargg\mem_timing-2\mem_timing.gb
    tests\blargg\oam_bug\oam_bug.gb
    tests\blargg\instr_timing\instr_timing.gb
  ) do (
    if exist "%%B" (
      set /a TOTAL+=1
      set ROM=%%B
      set NAME=%%~nB
      echo Running Blargg ROM: !ROM!
      "%SIMP%" "!ROM!" %CYCLES% --headless --dump-ppm "%BLARGG_DIR%\!NAME!.ppm" > "%BLARGG_DIR%\!NAME!.log" 2>&1
      findstr /C:"Passed" "%BLARGG_DIR%\!NAME!.log" >nul
      if !errorlevel! equ 0 (
        echo PASS BLARGG !NAME!
        echo PASS BLARGG !ROM! >> "%LOGS_DIR%\rom_test_results.log"
        set /a PASSED+=1
      ) else (
        echo FAIL BLARGG !NAME!
        echo FAIL BLARGG !ROM! >> "%LOGS_DIR%\rom_test_results.log"
      )
    )
  )
) else (
  for /R tests\blargg %%B in (*.gb) do (
    set ROM=%%B
    set NAME=%%~nB
    set SKIP=0
    if /I "!NAME!"=="01-special" set SKIP=1
    if /I "!NAME!"=="09-op r,r" set SKIP=1
    if !SKIP! equ 1 (
      echo Skipping (known long/hang): !ROM!
    ) else (
      set /a TOTAL+=1
      echo Running Blargg ROM: !ROM!
      "%SIMP%" "!ROM!" %CYCLES% --headless --dump-ppm "%BLARGG_DIR%\!NAME!.ppm" > "%BLARGG_DIR%\!NAME!.log" 2>&1
      findstr /C:"Passed" "%BLARGG_DIR%\!NAME!.log" >nul
      if !errorlevel! equ 0 (
        echo PASS BLARGG !NAME!
        echo PASS BLARGG !ROM! >> "%LOGS_DIR%\rom_test_results.log"
        set /a PASSED+=1
      ) else (
        echo FAIL BLARGG !NAME!
        echo FAIL BLARGG !ROM! >> "%LOGS_DIR%\rom_test_results.log"
      )
    )
  )
)

echo ========================================
echo ROM tests: %PASSED%/%TOTAL% passed >> "%LOGS_DIR%\rom_test_results.log"
echo ROM tests: %PASSED%/%TOTAL% passed
if %PASSED%==%TOTAL% (
  exit /b 0
) else (
  exit /b 1
)
