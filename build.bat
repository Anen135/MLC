@echo off
setlocal

set "ROOT=%~dp0"
set "OUTDIR=%ROOT%build"
set "TARGET=%OUTDIR%\mlc.exe"

pushd "%ROOT%" >nul

if not exist "%OUTDIR%" mkdir "%OUTDIR%"

where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo Building %TARGET% with gcc...
    gcc -std=c11 -Wall -Wextra -Isrc src\main.c src\compiler.c src\processor.c src\utils.c -o "%TARGET%"
    if errorlevel 1 goto failed
    goto success
)

where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo Building %TARGET% with cl...
    cl /nologo /W4 /I src src\main.c src\compiler.c src\processor.c src\utils.c /Fo"%OUTDIR%\\" /Fe:"%TARGET%"
    if errorlevel 1 goto failed
    goto success
)

echo Error: no supported C compiler found.
echo Install GCC, or run this from a Visual Studio Developer Command Prompt.
popd >nul
exit /b 1

:success
echo Built %TARGET%
popd >nul
exit /b 0

:failed
echo Build failed.
popd >nul
exit /b 1
