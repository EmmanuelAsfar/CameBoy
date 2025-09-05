@echo off
echo ========================================
echo Analyse des resultats Blargg
echo ========================================

set "result_dir=blargg_results"

echo.
echo Recherche de sortie port serie dans tous les tests...
echo.

for %%f in ("%result_dir%\*.txt") do (
    echo Test: %%~nxf
    echo ----------------------------------------
    
    findstr /C:"Port série" "%%f" >nul 2>&1
    if %errorlevel% equ 0 (
        echo [SUCCES] Port serie detecte !
        findstr /C:"Port série" "%%f" | head -3
    ) else (
        findstr /C:"0xFF01" "%%f" >nul 2>&1
        if %errorlevel% equ 0 (
            echo [SUCCES] Ecriture 0xFF01 detectee !
            findstr /C:"0xFF01" "%%f" | head -3
        ) else (
            findstr /C:"0xFF02" "%%f" >nul 2>&1
            if %errorlevel% equ 0 (
                echo [SUCCES] Ecriture 0xFF02 detectee !
                findstr /C:"0xFF02" "%%f" | head -3
            ) else (
                echo [ECHEC] Aucune sortie port serie
                echo Dernieres lignes:
                powershell "Get-Content '%%f' | Select-Object -Last 3"
            )
        )
    )
    echo.
)

echo ========================================
echo Analyse terminee !
echo ========================================
