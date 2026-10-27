@echo off
rem Build CFITSIO as a static library (/MD, like PCL) against PCL's zlib. No curl, no threads, no tools.
rem On first run the CFITSIO 4.7.0 source is downloaded from HEASARC's GitHub, its SHA-256 checked,
rem and unpacked into cfitsio\ (it is not in this repository). Run build_pcl.cmd first.
setlocal
set ROOT=%~dp0
set CFITSIO_URL=https://github.com/HEASARC/cfitsio/archive/refs/tags/cfitsio-4.7.0.tar.gz
set CFITSIO_SHA256=F281CA292407D8682E83A37EA68FCE6E1383D2449D923647EA62FA0570C59AEE
set TARBALL=%ROOT%download\cfitsio-4.7.0.tar.gz
if not exist "%ROOT%cfitsio\fitsio.h" (
  if not exist "%ROOT%download" mkdir "%ROOT%download"
  if not exist "%TARBALL%" (
    echo === downloading %CFITSIO_URL%
    powershell -NoProfile -Command "Invoke-WebRequest -UseBasicParsing -Uri '%CFITSIO_URL%' -OutFile '%TARBALL%'" || exit /b 1
  )
  powershell -NoProfile -Command "if ((Get-FileHash '%TARBALL%' -Algorithm SHA256).Hash -ne '%CFITSIO_SHA256%') { Write-Host 'CFITSIO download has the wrong SHA-256'; exit 1 }" || exit /b 1
  if not exist "%ROOT%cfitsio" mkdir "%ROOT%cfitsio"
  tar -xzf "%TARBALL%" -C "%ROOT%cfitsio" --strip-components=1 || exit /b 1
)
set CMAKE="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
%CMAKE% -S "%ROOT%cfitsio" -B "%ROOT%cfitsio\build" -G "Visual Studio 17 2022" -A x64 ^
  -DBUILD_SHARED_LIBS=OFF -DUSE_CURL=OFF -DUSE_PTHREADS=OFF -DTESTS=OFF -DUTILS=OFF -DITERPROGS=OFF ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DCMAKE_POLICY_DEFAULT_CMP0091=NEW ^
  -DZLIB_INCLUDE_DIR="%ROOT%PCL\src\3rdparty\zlib" -DZLIB_LIBRARY="%ROOT%PCL\lib\x64\zlib-pxi.lib" || exit /b 1
%CMAKE% --build "%ROOT%cfitsio\build" --config Release --target cfitsio || exit /b 1
echo CFITSIO OK
