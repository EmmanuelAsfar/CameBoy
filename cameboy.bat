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
if "%1"=="testrom" goto testrom
if "%1"=="build" goto build
if "%1"=="run" goto run
if "%1"=="gui" goto gui
if "%1"=="help" goto help
if "%1"=="examples" goto examples

REM Par défaut : build seulement (pas de GUI/ROM ni tests automatiques)
goto build

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
set "CFLAGS=-Wall -Wextra -std=c99 -O2 -g -Isrc -Ires"
set "LDFLAGS=-lgdi32 -luser32 -lkernel32"
set "BUILD_LOG=%BUILD_DIR%\build.log"

REM Construire dynamiquement la liste des sources
set "SOURCES="
for %%f in (src\*.c) do (
    if not "%%~nf"=="emulator_win32_gui" if not "%%~nf"=="graphics_win32_gui" if not "%%~nf"=="resource_manager" (
        set "SOURCES=!SOURCES! %%f"
    )
)

REM Construire dynamiquement la liste des sources GUI
set "GUI_SOURCES="
for %%f in (src\*.c) do (
    set "GUI_SOURCES=!GUI_SOURCES! %%f"
)

echo ======================================== > "%BUILD_LOG%"
echo Build started %DATE% %TIME% >> "%BUILD_LOG%"

REM S'assurer que le dossier bin existe
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%" 2>nul

REM Eviter "Acces refuse" lors de l'ecrasement si un exe tourne encore
taskkill /F /IM cameboy.exe >nul 2>&1
taskkill /F /IM cameboy_gui.exe >nul 2>&1

echo Compilation version simple...
echo Compilation version simple... >> "%BUILD_LOG%"
gcc %CFLAGS% %SOURCES% -o "%EXE%" %LDFLAGS% >> "%BUILD_LOG%" 2>&1
if errorlevel 1 (
    echo Build FAILED at %DATE% %TIME% >> "%BUILD_LOG%"
    echo ERREUR de compilation version simple
    type "%BUILD_LOG%"
    exit /b 1
)

echo Compilation version GUI...
echo Compilation version GUI... >> "%BUILD_LOG%"
gcc %CFLAGS% src\cpu.c src\cpu_tables.c src\cpu_tables_cb.c src\mmu.c src\timer.c src\ppu.c src\joypad.c src\interrupt.c src\apu.c src\resource_manager.c src\graphics_win32_gui.c src\emulator_win32_gui.c -o "%BIN_DIR%\cameboy_gui.exe" %LDFLAGS% -lcomctl32 -lshlwapi >> "%BUILD_LOG%" 2>&1
if errorlevel 1 (
    echo Build FAILED at %DATE% %TIME% >> "%BUILD_LOG%"
    echo ERREUR de compilation version GUI
    type "%BUILD_LOG%"
    exit /b 1
)

REM Copier ressources (BMP/PNG/etc.) dans le dossier build/resources
if not exist "%BUILD_DIR%\resources" mkdir "%BUILD_DIR%\resources" 2>nul
if exist "%PROJECT_DIR%resources" (
    echo Copie resources vers build\resources
    xcopy /E /I /Y "%PROJECT_DIR%resources" "%BUILD_DIR%\resources"
    echo Fin copie resources - errorlevel=!errorlevel!
)

if exist "%EXE%" (
    echo Build SUCCES ^(RUN^) at %DATE% %TIME% >> "%BUILD_LOG%"
) else (
    echo ERREUR: Executable RUN non trouve >> "%BUILD_LOG%"
    echo ERREUR: Executable RUN non trouve
    exit /b 1
)

if exist "%BIN_DIR%\cameboy_gui.exe" (
    echo Build SUCCES ^(GUI^) at %DATE% %TIME% >> "%BUILD_LOG%"
    echo ======================================== >> "%BUILD_LOG%"
    echo.
    echo SUCCES: Executables crees - %EXE% et %BIN_DIR%\cameboy_gui.exe
    echo.
) else (
    echo ERREUR: Executable GUI non trouve >> "%BUILD_LOG%"
    echo ERREUR: Executable GUI non trouve
    exit /b 1
)
goto end

:test
echo ========================================
echo TESTS UNITAIRES
echo ========================================
echo.

if not exist "%EXE%" (
    echo L'emulateur n'est pas compile. Lancez d'abord:cameboy.bat build
    exit /b 1
)

echo Lancement des tests...

REM Compiler et lancer les tests directement
set "TEST_BUILD_LOG=%LOGS_DIR%\test_build.log"
echo ======================================== >> "%TEST_BUILD_LOG%"
echo Test build started %DATE% %TIME% >> "%TEST_BUILD_LOG%"

REM S'assurer que le dossier bin existe
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%" 2>nul

REM Compiler dynamiquement tous les tests unitaires
for %%t in (tests\unit\test_*.c) do (
    set "TEST_NAME=%%~nt"
    echo Compilation !TEST_NAME!...
    
    REM Construire dynamiquement les dépendances pour chaque test (exclure les fichiers GUI et émulateur)
    set "TEST_DEPS="
    for %%s in (src\*.c) do (
        if not "%%~ns"=="emulator_win32_gui" if not "%%~ns"=="graphics_win32_gui" if not "%%~ns"=="graphics_win32_gui_clean" if not "%%~ns"=="graphics_win32_gui_old" if not "%%~ns"=="resource_manager" if not "%%~ns"=="emulator_simple" if not "%%~ns"=="emulator_win32" if not "%%~ns"=="graphics_win32" if not "%%~ns"=="png_loader" (
            set "TEST_DEPS=!TEST_DEPS! %%s"
        )
    )
    
    gcc %CFLAGS% %%t !TEST_DEPS! -o "%BIN_DIR%\!TEST_NAME!.exe" %LDFLAGS% 2>> "%TEST_BUILD_LOG%"
    if errorlevel 1 (
        echo ERREUR compilation !TEST_NAME!
        echo FAIL: !TEST_NAME! compilation at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
    ) else (
        echo OK: !TEST_NAME! compiled at %DATE% %TIME% >> "%TEST_BUILD_LOG%"
    )
)
echo ======================================== > "%LOGS_DIR%\test_results.log"
echo CameBoy Unit Tests - %DATE% %TIME% >> "%LOGS_DIR%\test_results.log"
echo ======================================== >> "%LOGS_DIR%\test_results.log"
echo. >> "%LOGS_DIR%\test_results.log"

set total=0
set passed=0

REM Executer dynamiquement tous les tests unitaires
for %%t in (tests\unit\test_*.c) do (
    set "TEST_NAME=%%~nt"
    if exist "%BIN_DIR%\!TEST_NAME!.exe" (
        echo Running !TEST_NAME!...
        echo Running !TEST_NAME!... >> "%LOGS_DIR%\test_results.log"

        "%BIN_DIR%\!TEST_NAME!.exe" >> "%LOGS_DIR%\test_results.log" 2>&1
        if !errorlevel! equ 0 (
            echo SUCCES !TEST_NAME! PASSED
            echo SUCCES !TEST_NAME! PASSED >> "%LOGS_DIR%\test_results.log"
            set /a passed+=1
        ) else (
            echo ERREUR !TEST_NAME! FAILED
            echo ERREUR !TEST_NAME! FAILED >> "%LOGS_DIR%\test_results.log"
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
    set "TEST_STATUS=✅ TOUS LES TESTS REUSSIS"
    set "TEST_STATUS_CLASS=success"
) else (
    echo CERTAINS TESTS ONT ECHOUE
    echo CERTAINS TESTS ONT ECHOUE >> "%LOGS_DIR%\test_results.log"
    set "TEST_STATUS=❌ CERTAINS TESTS ONT ECHOUE"
    set "TEST_STATUS_CLASS=failure"
)

REM Generer le fichier markdown des resultats
echo Generation du rapport markdown...
call :generate_test_report
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

:testrom
echo ========================================
echo TESTS ROM (SERIAL)
echo ========================================
echo.
call tests\rom\run_rom_tests.bat
if errorlevel 1 (
    echo CERTAINS TESTS ROM ONT ECHOUE
    exit /b 1
) else (
    echo TOUS LES TESTS ROM SONT PASSES
)
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
    call :select_rom
    if errorlevel 1 exit /b 1
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

:gui
echo ========================================
echo LANCEMENT EMULATEUR GUI
echo ========================================
echo.

if not exist "%BIN_DIR%\cameboy_gui.exe" (
    echo L'emulateur GUI n'est pas compile. Lancez d'abord: cameboy.bat build
    exit /b 1
)

REM Chercher une ROM
if "%2"=="" (
    call :select_rom
    if errorlevel 1 exit /b 1
) else (
    set "ROM=%2"
)

if not exist "%ROM%" (
    echo ROM non trouvee: %ROM%
    exit /b 1
)

echo Lancement GUI: %ROM%
echo.
echo Controles clavier:
echo   Fleches: D-pad
echo   Z: Bouton A
echo   X: Bouton B
echo   Entree: Start
echo   Maj droit: Select
echo   Echap: Quitter
echo.

REM Lancer l'émulateur GUI
"%BIN_DIR%\cameboy_gui.exe" "%ROM%" 2> "%LOGS_DIR%\emulator_gui.log"

if errorlevel 1 (
    echo.
    echo Erreur lors de l'execution GUI
    echo Voir: %LOGS_DIR%\emulator_gui.log
    type "%LOGS_DIR%\emulator_gui.log"
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
echo   build       - Compile seulement (versions simple et GUI)
echo   test        - Lance les tests (besoin de build d'abord)
echo   run [rom]   - Lance l'emulateur simple avec une ROM
echo   gui [rom]   - Lance l'emulateur GUI avec une ROM
echo   clean       - Nettoie les fichiers generes
echo   help        - Affiche cette aide
echo   examples    - Compile les ROMs d'exemple (RGBDS/GBDK)
echo.
echo Exemples:
echo   cameboy.bat                    # Build + test
echo   cameboy.bat run test.gb        # Lance version simple avec ROM
echo   cameboy.bat gui test.gb        # Lance version GUI avec ROM
echo   cameboy.bat gui                # Cherche automatiquement une ROM pour GUI
echo.
echo Fichiers de logs:
echo   %LOGS_DIR%\build.log           - Compilation
echo   %LOGS_DIR%\test_results.log    - Resultats tests
echo   %LOGS_DIR%\emulator.log        - Execution emulateur simple
echo   %LOGS_DIR%\emulator_gui.log    - Execution emulateur GUI
echo.
echo Structure:
echo   build\bin\cameboy.exe         - Emulateur simple
echo   build\bin\cameboy_gui.exe     - Emulateur GUI complet
echo   logs\                         - Tous les logs
echo.
goto end

:select_rom
echo ========================================
echo SELECTION DE ROM
echo ========================================
echo.

REM Scanner tous les fichiers .gb dans le projet
set "ROM_COUNT=0"

echo Recherche des ROMs .gb...
for /r "%PROJECT_DIR%" %%f in (*.gb) do (
    set /a ROM_COUNT+=1
    REM Calculer le chemin relatif
    set "FULL_PATH=%%f"
    set "REL_PATH=!FULL_PATH:%PROJECT_DIR%=!"
    echo [!ROM_COUNT!] !REL_PATH!
)

if %ROM_COUNT%==0 (
    echo Aucune ROM .gb trouvee dans le projet
    echo Placez des ROMs .gb dans le projet ou specifiez le chemin
    echo Usage: cameboy.bat run chemin\vers\rom.gb
    exit /b 1
)

echo.
echo Selectionnez une ROM (1-%ROM_COUNT%) ou 0 pour annuler:
set /p "CHOICE=Votre choix: "

if "%CHOICE%"=="0" (
    echo Annule
    exit /b 1
)

REM Valider le choix
if %CHOICE% LSS 1 (
    echo Choix invalide
    exit /b 1
)
if %CHOICE% GTR %ROM_COUNT% (
    echo Choix invalide
    exit /b 1
)

REM Extraire la ROM selectionnee en rescannant
set "CURRENT_INDEX=0"
for /r "%PROJECT_DIR%" %%f in (*.gb) do (
    set /a CURRENT_INDEX+=1
    if !CURRENT_INDEX!==%CHOICE% (
        set "ROM=%%f"
        goto :rom_found
    )
)

:rom_found
REM Afficher le chemin relatif
set "REL_ROM=!ROM:%PROJECT_DIR%=!"
echo ROM selectionnee: !REL_ROM!
echo.
exit /b 0

:end
echo.
echo ========================================
exit /b

:examples
echo ========================================
echo BUILD ROMs D'EXEMPLE
echo ========================================
echo.

REM Build RGBDS examples
if exist "examples\rgbds\build_all.bat" (
    echo [RGBDS] Building RGBDS examples...
    call examples\rgbds\build_all.bat
    echo.
)

REM Build GBDK examples
if exist "examples\gbdk\build_all.bat" (
    echo [GBDK] Building GBDK examples...
    call examples\gbdk\build_all.bat
    echo.
)

echo ========================================
echo BUILD EXEMPLES TERMINE
echo ========================================

goto end

:generate_test_report
REM Generer le fichier markdown des resultats de tests
set "TEST_MD=%PROJECT_DIR%TEST_RESULTS.md"
echo # Résultats des Tests Unitaires - CameBoy > "%TEST_MD%"
echo. >> "%TEST_MD%"
echo **Date:** %DATE% %TIME% >> "%TEST_MD%"
echo **Status:** %TEST_STATUS% >> "%TEST_MD%"
echo. >> "%TEST_MD%"

REM Calculer le pourcentage de reussite
if %total% gtr 0 (
    set /a "PERCENTAGE=100*%passed%/%total%"
) else (
    set "PERCENTAGE=0"
)
echo ## 📊 Synthèse Globale >> "%TEST_MD%"
echo. >> "%TEST_MD%"
echo ^| Métrique ^| Valeur ^| >> "%TEST_MD%"
echo ^|---------^|--------^| >> "%TEST_MD%"
echo ^| **Total des tests** ^| %total% ^| >> "%TEST_MD%"
echo ^| **Tests réussis** ^| %passed% ^| >> "%TEST_MD%"
set /a "FAILED=%total%-%passed%"
echo ^| **Tests échoués** ^| !FAILED! ^| >> "%TEST_MD%"
echo ^| **Taux de reussite** ^| %PERCENTAGE%%%% ^| >> "%TEST_MD%"
echo. >> "%TEST_MD%"

REM Compter les tests par rubrique
set "CPU_TESTS=0"
set "CPU_PASSED=0"
set "MMU_TESTS=0"
set "MMU_PASSED=0"
set "PPU_TESTS=0"
set "PPU_PASSED=0"
set "TIMER_TESTS=0"
set "TIMER_PASSED=0"
set "INTERRUPT_TESTS=0"
set "INTERRUPT_PASSED=0"
set "JOYPAD_TESTS=0"
set "JOYPAD_PASSED=0"

echo ## 📋 Résultats par Rubrique >> "%TEST_MD%"
echo. >> "%TEST_MD%"
echo ^| Rubrique ^| Tests ^| Réussis ^| Échecs ^| Status ^| >> "%TEST_MD%"
echo ^|----------^|-------^|---------^|--------^|--------^| >> "%TEST_MD%"

REM Analyser dynamiquement chaque test
for %%t in (tests\unit\test_*.c) do (
    set "TEST_NAME=%%~nt"
    if exist "%BIN_DIR%\!TEST_NAME!.exe" (
        set /a "!TEST_NAME!_TESTS+=1"
        "%BIN_DIR%\!TEST_NAME!.exe" >nul 2>&1
        if !errorlevel! equ 0 (
            set /a "!TEST_NAME!_PASSED+=1"
            REM Compter les sous-tests reels
            "%BIN_DIR%\!TEST_NAME!.exe" > "%TEMP%\!TEST_NAME!_output.txt" 2>&1
            set "!TEST_NAME!_SUBTESTS=0"
            for /f %%i in ('type "%TEMP%\!TEST_NAME!_output.txt" ^| findstr /c:"Test "') do set /a "!TEST_NAME!_SUBTESTS+=1"
            set "CURRENT_SUBTESTS=!TEST_NAME!_SUBTESTS!"
            echo ^| **!TEST_NAME!** ^| !CURRENT_SUBTESTS! ^| !CURRENT_SUBTESTS! ^| 0 ^| ✅ ^| >> "%TEST_MD%"
        ) else (
            REM Compter les sous-tests reels meme en cas d'echec
            "%BIN_DIR%\!TEST_NAME!.exe" > "%TEMP%\!TEST_NAME!_output.txt" 2>&1
            set "!TEST_NAME!_SUBTESTS=0"
            for /f %%i in ('type "%TEMP%\!TEST_NAME!_output.txt" ^| findstr /c:"Test "') do set /a "!TEST_NAME!_SUBTESTS+=1"
            set "!TEST_NAME!_FAILED_SUBTESTS=0"
            for /f %%i in ('type "%TEMP%\!TEST_NAME!_output.txt" ^| findstr /c:"FAIL"') do set /a "!TEST_NAME!_FAILED_SUBTESTS+=1"
            for /f %%i in ('type "%TEMP%\!TEST_NAME!_output.txt" ^| findstr /c:"Assertion failed"') do set /a "!TEST_NAME!_FAILED_SUBTESTS+=1"
            set /a "!TEST_NAME!_PASSED_SUBTESTS=!TEST_NAME!_SUBTESTS!-!TEST_NAME!_FAILED_SUBTESTS!"
            set "CURRENT_SUBTESTS=!TEST_NAME!_SUBTESTS!"
            set "CURRENT_PASSED=!TEST_NAME!_PASSED_SUBTESTS!"
            set "CURRENT_FAILED=!TEST_NAME!_FAILED_SUBTESTS!"
            echo ^| **!TEST_NAME!** ^| !CURRENT_SUBTESTS! ^| !CURRENT_PASSED! ^| !CURRENT_FAILED! ^| ❌ ^| >> "%TEST_MD%"
        )
    )
)

echo. >> "%TEST_MD%"
echo ## 🔍 Détails des Tests >> "%TEST_MD%"
echo. >> "%TEST_MD%"

REM Ajouter dynamiquement les details de chaque test
for %%t in (tests\unit\test_*.c) do (
    set "TEST_NAME=%%~nt"
    if exist "%BIN_DIR%\!TEST_NAME!.exe" (
        echo ### !TEST_NAME! >> "%TEST_MD%"
        echo. >> "%TEST_MD%"
        echo **Status:** >> "%TEST_MD%"
        "%BIN_DIR%\!TEST_NAME!.exe" >nul 2>&1
        if !errorlevel! equ 0 (
            echo ✅ **PASSED** >> "%TEST_MD%"
        ) else (
            echo ❌ **FAILED** >> "%TEST_MD%"
        )
        echo. >> "%TEST_MD%"
        echo **Log:** >> "%TEST_MD%"
        echo ``` >> "%TEST_MD%"
        "%BIN_DIR%\!TEST_NAME!.exe" >> "%TEST_MD%" 2>&1
        echo ``` >> "%TEST_MD%"
        echo. >> "%TEST_MD%"
    )
)


echo ## Logs Complets >> "%TEST_MD%"
echo. >> "%TEST_MD%"
echo Pour plus de détails, consultez le fichier de log complet: >> "%TEST_MD%"
echo - [test_results.log](logs/test_results.log) >> "%TEST_MD%"
echo. >> "%TEST_MD%"
echo --- >> "%TEST_MD%"
echo *Généré automatiquement par cameboy.bat* >> "%TEST_MD%"

echo Rapport markdown genere: %TEST_MD%
exit /b 0
