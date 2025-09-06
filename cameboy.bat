@echo off
REM =============================================================================
REM CameBoy - Script principal pour Windows
REM Compile, teste et lance l'émulateur
REM =============================================================================

setlocal enabledelayedexpansion

REM Configuration
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "BIN_DIR=%BUILD_DIR%\bin"
set "LOGS_DIR=%PROJECT_DIR%logs"
set "TEST_DIR=%PROJECT_DIR%tests"
set "EXE=%BIN_DIR%\cameboy.exe"

REM Création des dossiers
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%" 2>nul
if not exist "%LOGS_DIR%" mkdir "%LOGS_DIR%" 2>nul

REM Gestion des arguments
if "%1"=="clean" goto clean
if "%1"=="test" goto test
if "%1"=="build" goto build
if "%1"=="run" goto run
if "%1"=="help" goto help

REM Par défaut : build + test
goto build_test

:build
echo ========================================
echo COMPILATION CameBoy
echo ========================================
echo.

REM Vérifier que GCC existe
where gcc >nul 2>&1
if errorlevel 1 (
    echo ERREUR: gcc n'est pas installe
    echo Installez MinGW ou TDM-GCC
    exit /b 1
)

REM Compiler directement avec GCC
echo Compilation en cours...
set "CFLAGS=-Wall -Wextra -std=c99 -O2 -g -Isrc"
set "LDFLAGS=-lgdi32 -luser32 -lkernel32"
set "SOURCES=src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\interrupt.c src\apu.c src\graphics_win32.c src\emulator_simple.c"
set "BUILD_LOG=%LOGS_DIR%\build.log"

echo ======================================== > "%BUILD_LOG%"
echo Build started %DATE% %TIME% >> "%BUILD_LOG%"

REM S'assurer que le dossier bin existe
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%" 2>nul

gcc %CFLAGS% %SOURCES% -o "%EXE%" %LDFLAGS% >> "%BUILD_LOG%" 2>&1
if errorlevel 1 (
    echo Build FAILED at %DATE% %TIME% >> "%BUILD_LOG%"
    echo ERREUR de compilation
    type "%BUILD_LOG%"
    exit /b 1
)

if exist "%EXE%" (
    echo Build SUCCES at %DATE% %TIME% >> "%BUILD_LOG%"
    echo ======================================== >> "%BUILD_LOG%"
    echo.
    echo SUCCES: Executable cree - %EXE%
    echo.
) else (
    echo ERREUR: Executable non trouve
    exit /b 1
)
goto end

:test
echo ========================================
echo TESTS UNITAIRES
echo ========================================
echo.

if not exist "%EXE%" (
    echo L'emulateur n'est pas compile. Lancez d'abord: cameboy.bat build
    exit /b 1
)

echo Lancement des tests...

REM Compiler et lancer les tests directement
set "TEST_BUILD_LOG=%LOGS_DIR%\test_build.log"
echo ======================================== >> "%TEST_BUILD_LOG%"
echo Test build started %DATE% %TIME% >> "%TEST_BUILD_LOG%"

REM S'assurer que le dossier bin existe
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%" 2>nul

echo Compilation test_cpu...
gcc %CFLAGS% tests\unit\test_cpu.c src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\apu.c -o "%BIN_DIR%\test_cpu.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
if errorlevel 1 (
    echo ERREUR compilation test_cpu
    echo FAIL: test_cpu compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
) else (
    echo OK: test_cpu compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
)

echo Compilation test_mmu...
gcc %CFLAGS% tests\unit\test_mmu.c src\mmu.c src\timer.c src\apu.c -o "%BIN_DIR%\test_mmu.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
if errorlevel 1 (
    echo ERREUR compilation test_mmu
    echo FAIL: test_mmu compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
) else (
    echo OK: test_mmu compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
)

echo Compilation test_ppu...
gcc %CFLAGS% tests\unit\test_ppu.c src\ppu.c -o "%BIN_DIR%\test_ppu.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
if errorlevel 1 (
    echo ERREUR compilation test_ppu
    echo FAIL: test_ppu compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
) else (
    echo OK: test_ppu compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
)

echo Compilation test_timer...
gcc %CFLAGS% tests\unit\test_timer.c src\timer.c -o "%BIN_DIR%\test_timer.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
if errorlevel 1 (
    echo ERREUR compilation test_timer
    echo FAIL: test_timer compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
) else (
    echo OK: test_timer compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
)

echo Compilation test_interrupt...
gcc %CFLAGS% tests\unit\test_interrupt.c src\interrupt.c src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\apu.c -o "%BIN_DIR%\test_interrupt.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
if errorlevel 1 (
    echo ERREUR compilation test_interrupt
    echo FAIL: test_interrupt compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
) else (
    echo OK: test_interrupt compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
)

echo Compilation test_joypad...
gcc %CFLAGS% tests\unit\test_joypad.c src\joypad.c -o "%BIN_DIR%\test_joypad.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
if errorlevel 1 (
    echo ERREUR compilation test_joypad
    echo FAIL: test_joypad compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
) else (
    echo OK: test_joypad compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
)

echo ======================================== > "%LOGS_DIR%\test_results.log"
echo CameBoy Unit Tests - %DATE% %TIME% >> "%LOGS_DIR%\test_results.log"
echo ======================================== >> "%LOGS_DIR%\test_results.log"
echo. >> "%LOGS_DIR%\test_results.log"

set total=0
set passed=0

for %%t in (cpu mmu ppu timer interrupt joypad) do (
    if exist "%BIN_DIR%\test_%%t.exe" (
        echo Running test_%%t...
        echo Running test_%%t... >> "%LOGS_DIR%\test_results.log"

        "%BIN_DIR%\test_%%t.exe" >> "%LOGS_DIR%\test_results.log" 2>&1
        if !errorlevel! equ 0 (
            echo SUCCES test_%%t PASSED
            echo SUCCES test_%%t PASSED >> "%LOGS_DIR%\test_results.log"
            set /a passed+=1
        ) else (
            echo ERREUR test_%%t FAILED
            echo ERREUR test_%%t FAILED >> "%LOGS_DIR%\test_results.log"
        )
        set /a total+=1
        echo. >> "%LOGS_DIR%\test_results.log"
    )
)

echo ======================================== >> "%LOGS_DIR%\test_results.log"
echo Total: %passed%/%total% tests passed >> "%LOGS_DIR%\test_results.log"

if %passed%==%total% (
    echo TOUS LES TESTS REUSSIS!
    echo TOUS LES TESTS REUSSIS! >> "%LOGS_DIR%\test_results.log"
) else (
    echo CERTAINS TESTS ONT ECHOUE
    echo CERTAINS TESTS ONT ECHOUE >> "%LOGS_DIR%\test_results.log"
)
if errorlevel 1 (
    echo.
    echo Certains tests ont echoue
    echo Voir: %LOGS_DIR%\test_results.log
) else (
    echo.
    echo Tous les tests sont passes!
)
echo.
echo Test build ended %DATE% %TIME% >> "%TEST_BUILD_LOG%"
echo ======================================== >> "%TEST_BUILD_LOG%"
goto end

:run
echo ========================================
echo LANCEMENT EMULATEUR
echo ========================================
echo.

if not exist "%EXE%" (
    echo L'emulateur n'est pas compile. Lancez d'abord: cameboy.bat build
    exit /b 1
)

REM Chercher une ROM
if "%2"=="" (
    REM Chercher des ROMs dans tests/
    if exist "%TEST_DIR%\test_rom.gb" (
        set "ROM=%TEST_DIR%\test_rom.gb"
    ) else if exist "%TEST_DIR%\blargg\cpu_instrs\individual\01-special.gb" (
        set "ROM=%TEST_DIR%\blargg\cpu_instrs\individual\01-special.gb"
    ) else (
        echo Aucune ROM trouvee
        echo Placez une ROM .gb dans %TEST_DIR% ou specifie le chemin
        echo Usage: cameboy.bat run chemin\vers\rom.gb
        exit /b 1
    )
) else (
    set "ROM=%2"
)

if not exist "%ROM%" (
    echo ROM non trouvee: %ROM%
    exit /b 1
)

echo Lancement: %ROM%
echo.

REM Lancer l'émulateur
"%EXE%" "%ROM%" 2> "%LOGS_DIR%\emulator.log"

if errorlevel 1 (
    echo.
    echo Erreur lors de l'execution
    echo Voir: %LOGS_DIR%\emulator.log
    type "%LOGS_DIR%\emulator.log"
)

goto end

:build_test
call :build
if errorlevel 1 goto end
call :test
goto end

:clean
echo ========================================
echo NETTOYAGE
echo ========================================
echo.
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%" 2>nul
if exist "%LOGS_DIR%\*.log" del /q "%LOGS_DIR%\*.log" 2>nul
echo Nettoyage termine
goto end

:help
echo CameBoy - Script principal
echo ==========================
echo.
echo Usage: cameboy.bat [commande] [options]
echo.
echo Commandes:
echo   (rien)      - Compile et lance les tests
echo   build       - Compile seulement
echo   test        - Lance les tests (besoin de build d'abord)
echo   run [rom]   - Lance l'emulateur avec une ROM
echo   clean       - Nettoie les fichiers generes
echo   help        - Affiche cette aide
echo.
echo Exemples:
echo   cameboy.bat                    # Build + test
echo   cameboy.bat run test.gb        # Lance avec une ROM specifique
echo   cameboy.bat run                # Cherche automatiquement une ROM
echo.
echo Fichiers de logs:
echo   %LOGS_DIR%\build.log           - Compilation
echo   %LOGS_DIR%\test_results.log    - Resultats tests
echo   %LOGS_DIR%\emulator.log        - Execution emulatur
echo.
echo Structure:
echo   build\bin\                    - Executables
echo   logs\                         - Tous les logs
echo.
goto end

:end
echo.
echo ========================================
