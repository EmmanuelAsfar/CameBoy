@echo off
setlocal ENABLEDELAYEDEXPANSION

rem Detect project root (normalize to absolute path with trailing backslash)
for %%I in ("%~dp0..\..") do set "PROJECT_DIR=%%~fI\"

set "LOCAL_GBDK=%PROJECT_DIR%tools\gbdk\bin"
if exist "%LOCAL_GBDK%\lcc.exe" (
  set "LCC=%LOCAL_GBDK%\lcc.exe"
) else if defined GBDK_HOME (
  set "LCC=%GBDK_HOME%\bin\lcc.exe"
) else (
  set "LCC=lcc"
)

echo [GBDK] Tool lcc: "%LCC%"
echo [GBDK] Local GBDK bin: "%LOCAL_GBDK%"

pushd "%~dp0"
set "OUTDIR=build"
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo [GBDK] Compiling...
"%LCC%" -o "%OUTDIR%\gbdk_hello.gb" "src\main.c" || goto :error_popd

echo Success: "%CD%\%OUTDIR%\gbdk_hello.gb"
popd
exit /b 0

:error
echo Build failed (GBDK). >&2
exit /b 1

:error_popd
popd
goto error


