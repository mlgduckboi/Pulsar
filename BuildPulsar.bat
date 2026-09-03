@echo off
setlocal enabledelayedexpansion

REM ==== Compiler and tools ====
set "CC=.\compiler\mwcceppc.exe"
set "LINKER=.\KamekLinker\Kamek.exe"

REM ==== Project directories ====
set "ENGINE=.\KamekInclude"
set "GAMESOURCE=.\GameSource"
set "PULSAR=.\PulsarEngine"
set "BUILD=.\build"

REM ==== Create build directory if missing ====
if not exist "%BUILD%" mkdir "%BUILD%"

REM ==== Compiler flags ====
set "CFLAGS=-I- -i "%ENGINE%" -i "%GAMESOURCE%" -i "%PULSAR%" -opt all -inline auto -enum int -proc gekko -fp hard -sdata 0 -sdata2 0 -maxerrors 1 -func_align 4"

REM ==== Compile kamek.cpp ====
echo Compiling %ENGINE%\kamek.cpp
"%CC%" %CFLAGS% -c -o "%BUILD%\kamek.o" "%ENGINE%\kamek.cpp"
if errorlevel 1 goto :error

REM ==== Collect object files ====
set "OBJECTS="

REM ==== Compile all .cpp files in PULSAR ====
for /r "%PULSAR%" %%F in (*.cpp) do (
    set "SRC=%%F"
    set "BASE=%%~nF"
    set "OBJ=%BUILD%\!BASE!.o"

    set "NEEDBUILD=1"
    if exist "!OBJ!" (
        call :isnewer "!SRC!" "!OBJ!" NEEDBUILD
    )

    if "!NEEDBUILD!"=="1" (
        echo Compiling !SRC!
        "%CC%" %CFLAGS% -c -o "!OBJ!" "!SRC!"
        if errorlevel 1 goto :error
    ) else (
        echo Skipping !SRC! ^(up to date^)
    )

    set "OBJECTS=!OBJECTS! "!OBJ!""
)

REM ==== Compile all .s files in PULSAR ====
for /r "%PULSAR%" %%F in (*.s) do (
    set "SRC=%%F"
    set "BASE=%%~nF"
    set "OBJ=%BUILD%\!BASE!.o"

    set "NEEDBUILD=1"
    if exist "!OBJ!" (
        call :isnewer "!SRC!" "!OBJ!" NEEDBUILD
    )

    if "!NEEDBUILD!"=="1" (
        echo Compiling assembly !SRC!
        "%CC%" %CFLAGS% -c -o "!OBJ!" "!SRC!"
        if errorlevel 1 goto :error
    ) else (
        echo Skipping assembly !SRC! ^(up to date^)
    )

    set "OBJECTS=!OBJECTS! "!OBJ!""
)

REM ==== Link all object files ====
echo Linking...
"%LINKER%" "%BUILD%\kamek.o" %OBJECTS% ^
  -dynamic ^
  -externals="%GAMESOURCE%\symbols.txt" ^
  -versions="%GAMESOURCE%\versions.txt" ^
  -output-combined="%BUILD%\Code.pul"
if errorlevel 1 goto :error

echo Build succeeded.
goto :eof

REM =========================================================
REM :isnewer <source> <object> <resultVar>
REM Sets resultVar=1 if source is newer than object, else 0
REM Uses PowerShell for reliable timestamp comparison.
REM =========================================================
:isnewer
set "_SRC=%~1"
set "_OBJ=%~2"
for /f %%R in ('powershell -NoProfile -Command ^
  "if ((Get-Item -LiteralPath '%_SRC%').LastWriteTime -gt (Get-Item -LiteralPath '%_OBJ%').LastWriteTime) { '1' } else { '0' }"') do (
    set "%~3=%%R"
)
goto :eof

:error
echo Build failed.
exit /b 1