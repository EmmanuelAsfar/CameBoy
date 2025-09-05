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
    ) else (
        findstr /C:"0xFF01" "%%f" >nul 2>&1
        if %errorlevel% equ 0 (
            echo [SUCCES] Ecriture 0xFF01 detectee !
        ) else (
            findstr /C:"0xFF02" "%%f" >nul 2>&1
            if %errorlevel% equ 0 (
                echo [SUCCES] Ecriture 0xFF02 detectee !
            ) else (
                echo [ECHEC] Aucune sortie port serie
            )
        )
    )
    echo.
)

echo ========================================
echo Analyse terminee !
echo ========================================
