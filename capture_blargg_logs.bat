@echo off
echo === Capture des logs Blargg ===
echo.

echo Lancement du test Blargg avec capture des logs...
.\bin\cameboy_test.exe ".\tests\blargg\cpu_instrs\individual\01-special.gb" 1000000 > blargg_console_log.txt 2>&1

echo.
echo Analyse des logs...
echo.

echo === Recherche de serial output ===
findstr /i "SERIAL:" blargg_console_log.txt

echo.
echo === Recherche de messages de test ===
findstr /i "test\|passed\|failed\|error" blargg_console_log.txt

echo.
echo === Dernières lignes du log ===
powershell "Get-Content blargg_console_log.txt | Select-Object -Last 20"

echo.
echo Log complet sauvé dans: blargg_console_log.txt
