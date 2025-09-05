@echo off
echo Création de ROMs de test simples pour CameBoy...
echo.

REM Créer le dossier tests s'il n'existe pas
if not exist "tests" mkdir tests

echo Compilation du générateur de ROM...
"C:\TDM-GCC-64\bin\gcc.exe" tests\test_rom.c -o tests\create_test_rom.exe

if %errorlevel% == 0 (
    echo ✅ Générateur compilé
    
    echo Création de ROMs de test...
    .\tests\create_test_rom.exe
    
    echo.
    echo ROMs de test créées :
    echo - tests\test_rom.gb (ROM de base)
    echo.
    echo Pour tester :
    echo   .\bin\cameboy_window.exe .\tests\test_rom.gb
) else (
    echo ❌ Erreur de compilation
)

pause
