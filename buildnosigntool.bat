
@echo off
chcp 65001

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86

REM cd /d D:\repo\pvztoolkit
cd /d %~dp0

set INCLUDE=.\fltk\include;%INCLUDE%
set LIB=.\fltk\lib;%LIB%

set INCLUDE=.\zlib\include;%INCLUDE%
set LIB=.\zlib\lib;%LIB%

set INCLUDE=C:\dev\vcpkg\installed\x86-windows\include;%INCLUDE%
set LIB=C:\dev\vcpkg\installed\x86-windows\lib;%LIB%

if exist .\out\pvztoolkitd.exe del .\out\pvztoolkitd.exe
if exist .\out\pvztoolkit.exe nmake -f makefile.release clean

nmake -f makefile.debug

if not exist .\out\pvztoolkitd.exe goto :end

mt.exe -nologo ^
-manifest ".\res\ptk.manifest" ^
-outputresource:".\out\pvztoolkitd.exe;#1"

goto :end rem release

nmake -f makefile.release clean
nmake -f makefile.release

if not exist .\out\pvztoolkit.exe goto :end

mt.exe -nologo ^
-manifest ".\res\ptk.manifest" ^
-outputresource:".\out\pvztoolkit.exe;#1"

goto :end

file="PvZ_Toolkit_v1.20.3.exe"
rm $file.hash
echo $file >> $file.hash
echo "MD5" >> $file.hash
md5sum $file | cut -d" " -f1 | tr [a-z] [A-Z] >> $file.hash
echo "SHA-1" >> $file.hash
sha1sum $file | cut -d" " -f1 | tr [a-z] [A-Z] >> $file.hash
echo "SHA-256" >> $file.hash
sha256sum $file | cut -d" " -f1 | tr [a-z] [A-Z] >> $file.hash
echo "SHA-512" >> $file.hash
sha512sum $file | cut -d" " -f1 | tr [a-z] [A-Z] >> $file.hash

gpg --armor --detach-sign $file
gpg --verify $file.asc $file

:end

if not exist .\out\splash.png copy bin\splash.png .\out
if not exist .\out\lineup.yml copy bin\lineup.yml .\out
if not exist .\out\builds.yml copy bin\builds.yml .\out
