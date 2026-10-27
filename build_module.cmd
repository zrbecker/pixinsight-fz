@echo off
rem Compile and link FZ-pxm.dll with the same compiler settings as PCL (Release x64, /MD, AVX2, C++20).
setlocal
set ROOT=%~dp0
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
set OUT=%ROOT%module\build
if not exist "%OUT%" mkdir "%OUT%"
cl /nologo /c /O2 /MD /EHsc /std:c++20 /permissive- /Zc:__cplusplus /arch:AVX2 /W3 /wd4996 ^
   /DWIN32 /DWIN64 /D_WINDOWS /DUNICODE /D_UNICODE /D__PCL_WINDOWS /D__PCL_NO_WIN32_MINIMUM_VERSIONS ^
   /D__PCL_AVX2 /D__PCL_FMA /D_NDEBUG ^
   /I"%ROOT%PCL\include" /I"%ROOT%cfitsio" /I"%ROOT%cfitsio\build" ^
   /Fo"%OUT%\\" "%ROOT%module\FZModule.cpp" "%ROOT%module\FZFormat.cpp" "%ROOT%module\FZInstance.cpp" || exit /b 1
link /nologo /DLL /OUT:"%OUT%\FZ-pxm.dll" /LIBPATH:"%ROOT%PCL\lib\x64" ^
   "%OUT%\FZModule.obj" "%OUT%\FZFormat.obj" "%OUT%\FZInstance.obj" ^
   PCL-pxi.lib lz4-pxi.lib zlib-pxi.lib zstd-pxi.lib lcms-pxi.lib cminpack-pxi.lib RFC6234-pxi.lib ^
   "%ROOT%cfitsio\build\Release\cfitsio.lib" ^
   kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib ws2_32.lib bcrypt.lib userenv.lib || exit /b 1
echo MODULE OK
