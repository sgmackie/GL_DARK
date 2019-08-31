echo Clang x64

@echo off

:: Set name of the platform .cpp file for unity building
set Platform=win32_dark

:: Set build directory relative to current drive and path
set BuildDir="%~dp0..\build\win32"

:: Create build path if it doesn't exist
if not exist %BuildDir% mkdir %BuildDir%

:: Move to build directory
pushd %BuildDir%

:: Set compiler arguments
set PlatformFiles="%~dp0%Platform%.cpp"
set ObjDir=.\obj\

:: Create Object path if it doesn't exist
if not exist %ObjDir% mkdir %ObjDir%

:: Set compiler flags:
set CompilerFlags=-g -gcodeview -pedantic

:: Set warning labels:
set CommonWarnings=-Wall -Werror -Wno-writable-strings -Wno-gnu-anonymous-struct

:: Set Compiler optimsation level
REM set CompilerOpt=-O3 -march=native
set CompilerOpt=-O0

:: Set win32 libraries
set Libs=-lUser32.lib -lGdi32.lib -lWinmm.lib -lopengl32.lib -lglew32.lib

:: Run Clang compiler
clang %CompilerFlags% %CommonWarnings% %CompilerOpt% %Libs% %PlatformFiles% -o %Platform%.exe

:: Exit
popd