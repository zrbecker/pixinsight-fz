@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
cl /nologo /O2 /MD /I..\cfitsio /I..\cfitsio\build fzcheck.c ..\cfitsio\build\Release\cfitsio.lib ..\PCL\lib\x64\zlib-pxi.lib ws2_32.lib /Fe:fzcheck.exe >nul || exit /b 1
echo TEST BUILD OK
