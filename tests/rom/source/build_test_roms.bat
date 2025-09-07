@echo off
REM Minimal: list gen_*.c, compile each, run, and move .gb to parent

pushd "%~dp0"

echo ========================================
echo BUILD TEST ROMS (list + compile + run + move)
echo ========================================
echo.

where gcc >nul 2>&1
if errorlevel 1 (
  echo ERREUR: gcc non trouve
  popd
  exit /b 1
)

echo Fichiers gen_*.c trouves:
dir /b gen_*.c
echo.

for %%F in (gen_*.c) do (
  echo Compilation: %%F
  gcc -std=c99 -O2 "%%F" -o "%%~nF.exe"
  if errorlevel 1 (
    echo ERREUR compilation: %%F
    popd
    exit /b 1
  )
)

echo.
echo Execution des generateurs:
for %%E in (gen_*.exe) do (
  echo Execution: %%E
  "%%E"
  if errorlevel 1 (
    echo ERREUR execution: %%E
    popd
    exit /b 1
  )
)

echo.
echo ROMs .gb dans source\\ (avant deplacer):
dir /b *.gb 2>nul

for %%G in (*.gb) do (
  echo Deplacement: %%G ^> ..\\
  move /Y "%%G" "..\\" >nul
)

echo.
echo ROMs .gb dans tests\\rom\\ :
for %%H in (..\*.gb) do echo   - %%~nxH

echo.
echo OK.
popd
exit /b 0
