@echo off
REM Force UTF-8 code page for proper accent handling
chcp 65001 >nul
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

REM Build GBDK/RGBDS ROMs (if toolchains are present)
if exist tests\rom\source\build_roms_gbdk.bat (
  call tests\rom\source\build_roms_gbdk.bat
)
if exist tests\rom\source\build_roms_rgbds.bat (
  call tests\rom\source\build_roms_rgbds.bat
)

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

REM Legacy generators (generate_roms.c, gen_*.c) removed from flow

set TOTAL=0
set PASSED=0
set NIMPL=0
set DEFAULT_CYCLES=60000000

for %%R in (tests\rom\*.gb) do (
  set /a TOTAL+=1
  set ROM=%%R
  set NAME=%%~nR
  echo Running ROM: !ROM!
  set CYCLES=
  set SKIP=0
  rem Structured .expect can also include CONFIG.* overrides
  set TEST_COUNT=0
  set TEST_IDS=
  set ONLY_IDS=
  if exist tests\rom\config\!NAME!.expect (
    for /f "usebackq delims=" %%E in ("tests\rom\config\!NAME!.expect") do (
      set "LINE=%%E"
      if not "!LINE!"=="" if not "!LINE:~0,1!"=="#" if not "!LINE:~0,1!"==";" (
        for /f "tokens=1,2* delims==" %%K in ("!LINE!") do (
          set "KEY=%%K"
          set "VAL=%%L"
          rem Normalize key
          set "KUP=!KEY:~0,6!"
          if /I "!KEY!"=="CONFIG.CYCLES" set CYCLES=!VAL!
          if /I "!KEY!"=="CONFIG.SKIP" set SKIP=!VAL!
          if /I "!KEY!"=="CONFIG.ONLY" set ONLY_IDS=!VAL!
          rem TEST.<id>.* keys
          for /f "tokens=1-3 delims=." %%X in ("!KEY!") do (
            if /I "%%X"=="TEST" (
              set "ID=%%Y"
              set "FIELD=%%Z"
              if /I "!FIELD!"=="SERIAL" set "T_!ID!_SERIAL=!VAL!"
              if /I "!FIELD!"=="SERIAL_EXPECTED" set "T_!ID!_SERIAL=!VAL!"
              if /I "!FIELD!"=="SERIAL_SEQ" set "T_!ID!_SERIAL_SEQ=!VAL!"
              if /I "!FIELD!"=="SERIAL_COUNT" set "T_!ID!_SERIAL_COUNT=!VAL!"
              if /I "!FIELD!"=="CYCLES" set "T_!ID!_CYCLES=!VAL!"
              if /I "!FIELD!"=="DESC" set "T_!ID!_DESC=!VAL!"
              rem Back-compat: map old comment fields to DESC if DESC absent
              if /I "!FIELD!"=="COMMENT_PASS" if not defined T_!ID!_DESC set "T_!ID!_DESC=!VAL!"
              if /I "!FIELD!"=="COMMENT_FAIL" if not defined T_!ID!_DESC set "T_!ID!_DESC=!VAL!"
              if /I "!FIELD!"=="COMMENT_NI" if not defined T_!ID!_DESC set "T_!ID!_DESC=!VAL!"
              if /I "!FIELD!"=="TEST_COMMENT_PASS" if not defined T_!ID!_DESC set "T_!ID!_DESC=!VAL!"
              if /I "!FIELD!"=="TEST_COMMENT_FAIL" if not defined T_!ID!_DESC set "T_!ID!_DESC=!VAL!"
              if /I "!FIELD!"=="ENABLED" set "T_!ID!_ENABLED=!VAL!"
              if /I "!FIELD!"=="STATUS" set "T_!ID!_STATUS=!VAL!"
              rem Track ID when SERIAL is defined
              if defined T_!ID!_SERIAL (
                echo !TEST_IDS! | findstr /C:" !ID! " >nul || (
                  set /a TEST_COUNT+=1
                  set "TEST_IDS=!TEST_IDS! !ID! "
                )
              )
              rem Also track ID when STATUS=NOT_IMPLEMENTED/NI
              if defined T_!ID!_STATUS (
                for %%S in (!T_!ID!_STATUS!) do (
                  if /I "%%S"=="NOT_IMPLEMENTED" (
                    echo !TEST_IDS! | findstr /C:" !ID! " >nul || (
                      set /a TEST_COUNT+=1
                      set "TEST_IDS=!TEST_IDS! !ID! "
                    )
                  )
                  if /I "%%S"=="NI" (
                    echo !TEST_IDS! | findstr /C:" !ID! " >nul || (
                      set /a TEST_COUNT+=1
                      set "TEST_IDS=!TEST_IDS! !ID! "
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
  )
  if "!CYCLES!"=="" set CYCLES=%DEFAULT_CYCLES%
  if "!SKIP!"=="1" (
    echo SKIP !NAME!
    echo SKIP !NAME! >> "%LOGS_DIR%\rom_test_results.log"
  ) else (
  "%SIMP%" "!ROM!" !CYCLES! --headless --dump-ppm "%ROM_DIR%\!NAME!.ppm" > "%ROM_DIR%\!NAME!.log" 2>&1
  rem If emulator wrote a dedicated serial file, keep it; otherwise fall back to full log
  if exist "%ROM_DIR%\!NAME!_serial.txt" (
    rem keep existing serial capture
  ) else (
    type "%ROM_DIR%\!NAME!.log" > "%ROM_DIR%\!NAME!_serial.txt"
  )

    set ROM_PASSED=0
    set ROM_FAILED=0
    set ROM_NI=0
    if exist tests\rom\config\!NAME!.expect if !TEST_COUNT! gtr 0 (
      echo Checking structured expectations for !NAME!... >> "%LOGS_DIR%\rom_test_results.log"
      for %%I in (!TEST_IDS!) do (
        set "SID=%%I"
        set "SER="
        set "DESC="
        set "EN=1"
        set "ST="
        if defined T_!SID!_SERIAL set "SER=!T_!SID!_SERIAL!"
        if defined T_!SID!_DESC set "DESC=!T_!SID!_DESC!"
        if defined T_!SID!_ENABLED set "EN=!T_!SID!_ENABLED!"
        if defined T_!SID!_STATUS set "ST=!T_!SID!_STATUS!"
        if not "!EN!"=="0" (
          if /I "!ST!"=="NOT_IMPLEMENTED" (
            echo   [NI]   !NAME! :: TEST !SID! :: !SER! ^| !DESC! >> "%LOGS_DIR%\rom_test_results.log"
            set /a ROM_NI+=1
          ) else if /I "!ST!"=="NI" (
            echo   [NI]   !NAME! :: TEST !SID! :: !SER! ^| !DESC! >> "%LOGS_DIR%\rom_test_results.log"
            set /a ROM_NI+=1
          ) else if defined SER (
          findstr /C:"!SER!" "%ROM_DIR%\!NAME!_serial.txt" >nul
          if !errorlevel! equ 0 (
            echo   [PASS] !NAME! :: TEST !SID! :: !SER! ^| !DESC! >> "%LOGS_DIR%\rom_test_results.log"
            set /a ROM_PASSED+=1
          ) else (
            echo   [FAIL] !NAME! :: TEST !SID! :: !SER! ^| !DESC! >> "%LOGS_DIR%\rom_test_results.log"
            set /a ROM_FAILED+=1
          )
          )
        )
      )
      if !ROM_FAILED! equ 0 if !ROM_PASSED! gtr 0 (
        echo PASS !NAME!
        echo PASS !NAME! >> "%LOGS_DIR%\rom_test_results.log"
        set /a PASSED+=1
      ) else if !ROM_FAILED! equ 0 if !ROM_PASSED! equ 0 if !ROM_NI! gtr 0 (
        echo NI !NAME!
        echo NI !NAME! >> "%LOGS_DIR%\rom_test_results.log"
        set /a NIMPL+=1
      ) else (
        echo FAIL !NAME!
        echo FAIL !NAME! >> "%LOGS_DIR%\rom_test_results.log"
      )
    ) else if exist tests\rom\config\!NAME!.expect (
      echo Checking simple expectations for !NAME!... >> "%LOGS_DIR%\rom_test_results.log"
      for /f "usebackq delims=" %%E in ("tests\rom\config\!NAME!.expect") do (
        set "EXPECT_TOKEN=%%E"
        if not "!EXPECT_TOKEN!"=="" if not "!EXPECT_TOKEN:~0,1!"=="#" if not "!EXPECT_TOKEN:~0,1!"==";" (
          findstr /C:"!EXPECT_TOKEN!" "%ROM_DIR%\!NAME!_serial.txt" >nul
          if !errorlevel! equ 0 (
            echo   [PASS] !NAME! :: !EXPECT_TOKEN! >> "%LOGS_DIR%\rom_test_results.log"
            set /a ROM_PASSED+=1
          ) else (
            echo   [FAIL] !NAME! :: !EXPECT_TOKEN! >> "%LOGS_DIR%\rom_test_results.log"
            set /a ROM_FAILED+=1
          )
        )
      )
      if !ROM_FAILED! equ 0 (
        echo PASS !NAME!
        echo PASS !NAME! >> "%LOGS_DIR%\rom_test_results.log"
        set /a PASSED+=1
      ) else (
        echo FAIL !NAME!
        echo FAIL !NAME! >> "%LOGS_DIR%\rom_test_results.log"
      )
    ) else (
      rem Fallback: detect generic PASS in main log
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
  )
)

rem Blargg ROMs disabled for initial pipeline bring-up
rem echo --- Blargg ROMs (%MODE%) --- >> "%LOGS_DIR%\rom_test_results.log"
rem (skipped)

echo ========================================
echo ROM tests: %PASSED%/%TOTAL% passed >> "%LOGS_DIR%\rom_test_results.log"
echo ROM NI: %NIMPL% >> "%LOGS_DIR%\rom_test_results.log"
echo ROM tests: %PASSED%/%TOTAL% passed
echo ROM NI: %NIMPL%

REM Generate Markdown report mirroring unit tests (simple summary)
set "ROM_MD=%PROJECT_DIR%\ROM_RESULTS.md"
set /a FAILED=%TOTAL%-%PASSED%-%NIMPL%
echo ROM Test Results - CameBoy > "%ROM_MD%"
echo Date: %DATE% %TIME% >> "%ROM_MD%"
echo Status: %PASSED%/%TOTAL% passed >> "%ROM_MD%"
echo ROMs NI: %NIMPL% >> "%ROM_MD%"
echo ROMs FAIL: %FAILED% >> "%ROM_MD%"
echo. >> "%ROM_MD%"
echo --- rom_test_results.log excerpt --- >> "%ROM_MD%"
type "%LOGS_DIR%\rom_test_results.log" >> "%ROM_MD%"
echo Rapport ROM genere: %ROM_MD%

if %PASSED%==%TOTAL% (
  exit /b 0
) else (
  exit /b 1
)
