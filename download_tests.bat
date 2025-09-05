@echo off
echo Téléchargement des ROMs de test pour CameBoy...
echo.

echo Téléchargement des tests Blargg...
if not exist "tests\blargg" (
    git clone https://github.com/retrio/gb-test-roms tests\blargg
    if %errorlevel% == 0 (
        echo ✅ Tests Blargg téléchargés
    ) else (
        echo ❌ Erreur téléchargement Blargg
    )
) else (
    echo Tests Blargg déjà présents
)

echo.
echo Téléchargement des tests Mooneye...
if not exist "tests\mooneye" (
    git clone https://github.com/Gekkio/mooneye-test-suite tests\mooneye
    if %errorlevel% == 0 (
        echo ✅ Tests Mooneye téléchargés
    ) else (
        echo ❌ Erreur téléchargement Mooneye
    )
) else (
    echo Tests Mooneye déjà présents
)

echo.
echo ROMs de test disponibles :
echo - tests\blargg\cpu_instrs\01-special.gb
echo - tests\blargg\cpu_instrs\02-interrupts.gb
echo - tests\mooneye\acceptance\timer\tima_write_reloading.gb
echo - tests\mooneye\acceptance\timer\tima_reload.gb
echo.
echo Pour tester :
echo   .\bin\cameboy_window.exe .\tests\blargg\cpu_instrs\01-special.gb
echo.

pause
