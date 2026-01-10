@echo off
call "%~dp0tools\djgpp\setenv.bat"

make

rem set CFLAGS=-Wall -Werror -Werror -Wno-unused-function -O0 -march=i386 -mgeneral-regs-only -fomit-frame-pointer
rem set SRCS=src\main.c src\timer.c src\timer.s
rem i586-pc-msdosdjgpp-gcc %CFLAGS% -o build\myprog.exe %SRCS%

rem echo Build finished – executable in build\myprog.exe
