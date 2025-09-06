@echo off
echo === Compilation ROM animation ===
echo.

echo Compilation create_simple_animation...
gcc -Wall -Wextra -std=c99 -O2 -g create_simple_animation.c -o create_simple_animation.exe > animation_compilation_log.txt 2>&1

if %errorlevel% neq 0 (
    echo ❌ Erreur de compilation - Voir animation_compilation_log.txt
    type animation_compilation_log.txt
    exit /b 1
)

echo ✅ Compilation réussie !
echo.

echo Création de la ROM...
.\create_simple_animation.exe > animation_rom_log.txt 2>&1

if %errorlevel% neq 0 (
    echo ❌ Erreur création ROM - Voir animation_rom_log.txt
    type animation_rom_log.txt
    exit /b 1
)

echo ✅ ROM créée !
echo.

echo Test de la ROM avec l'émulateur...
.\bin\cameboy_test.exe simple_animation.gb 1000000 > animation_test_log.txt 2>&1

echo.
echo Résultats dans animation_test_log.txt
echo Dernières lignes:
powershell "Get-Content animation_test_log.txt | Select-Object -Last 10"
