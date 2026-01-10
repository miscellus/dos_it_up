@echo off
rem -------------------------------------------------
rem Run the compiled DOS program in DOSBox
rem -------------------------------------------------
set PATH=C:\Program Files (x86)\DOSBox-0.74-3;%PATH%
DosBOX.exe -c "mount c %~dp0build" -c "c:" -c "myprog.exe" -c "pause" -c "exit"
