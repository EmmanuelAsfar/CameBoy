@echo off
REM Script pour lancer l'émulateur avec interface graphique complète

if "%1"=="" (
    echo Usage: %0 ^<rom_file^>
    echo.
    echo Contrôles clavier:
    echo   Flèches: D-pad
    echo   Z: Bouton A
    echo   X: Bouton B
    echo   Entrée: Start
    echo   Maj droit: Select
    echo   Échap: Quitter
    echo.
    echo Exemple: %0 tests\rom\visual_grid.gb
    pause
    exit /b 1
)

echo Lancement de l'émulateur GUI avec ROM: %1
echo.

REM Vérifier que l'exécutable existe
if not exist "..\build\bin\cameboy_gui.exe" (
    echo Erreur: cameboy_gui.exe non trouvé. Exécutez d'abord 'build' ou 'make all'.
    pause
    exit /b 1
)

REM Vérifier que la ROM existe
if not exist "%1" (
    echo Erreur: ROM '%1' non trouvée.
    pause
    exit /b 1
)

REM Lancer l'émulateur GUI
cd /d "%~dp0"
..\build\bin\cameboy_gui.exe %1

echo.
echo Émulateur fermé.
pause
