@echo off
rem Build PCL and its bundled third-party libraries (Release x64) from the sources PixInsight ships.
rem The sources are copied from the PixInsight install into PCL\ on first run (they are not in this
rem repository). Set PIXINSIGHT_DIR if PixInsight is not in the default folder.
setlocal
if not defined PIXINSIGHT_DIR set PIXINSIGHT_DIR=C:\Program Files\PixInsight
set PCLDIR=%~dp0PCL
set PCLSRCDIR=%PCLDIR%\src
set PCLINCDIR=%PCLDIR%\include
set PCLLIBDIR64=%PCLDIR%\lib\x64
set PCLBINDIR64=%PCLDIR%\bin
if not exist "%PCLSRCDIR%\pcl\windows\vc17\PCL.vcxproj" (
  if not exist "%PIXINSIGHT_DIR%\src\pcl" ( echo PCL sources not found in "%PIXINSIGHT_DIR%\src". Set PIXINSIGHT_DIR. & exit /b 1 )
  echo === copying PCL sources from %PIXINSIGHT_DIR%
  robocopy "%PIXINSIGHT_DIR%\src" "%PCLSRCDIR%" /E /NJH /NJS /NFL /NDL /NP >nul
  robocopy "%PIXINSIGHT_DIR%\include" "%PCLINCDIR%" /E /NJH /NJS /NFL /NDL /NP >nul
)
if not exist "%PCLLIBDIR64%" mkdir "%PCLLIBDIR64%"
if not exist "%PCLBINDIR64%" mkdir "%PCLBINDIR64%"
set MSB="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
for %%P in (zlib lz4 zstd lcms cminpack RFC6234) do (
  echo === %%P
  %MSB% "%PCLSRCDIR%\3rdparty\%%P\windows\vc17\%%P.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal || exit /b 1
)
echo === PCL
%MSB% "%PCLSRCDIR%\pcl\windows\vc17\PCL.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal || exit /b 1
echo BUILD OK
