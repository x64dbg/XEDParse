@echo off
set RELEASEDIR=.\release
rmdir /S /Q %RELEASEDIR%
mkdir %RELEASEDIR%
mkdir %RELEASEDIR%\x32
mkdir %RELEASEDIR%\x64

copy src\XEDParse.h %RELEASEDIR%\XEDParse.h
xcopy bindings %RELEASEDIR% /S /Y
copy bin\x32\XEDParse.dll %RELEASEDIR%\x32\XEDParse.dll
copy bin\x32\XEDParse.lib %RELEASEDIR%\XEDParse_x86.lib
copy bin\x64\XEDParse.dll %RELEASEDIR%\x64\XEDParse.dll
copy bin\x64\XEDParse.lib %RELEASEDIR%\XEDParse_x64.lib

exit 0