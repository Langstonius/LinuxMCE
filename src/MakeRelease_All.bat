@echo on

echo Deleting old pluto.makerelease folder...
rmdir "/pluto.makerelease" /S /Q

echo Checking out pluto sources...
cd "c:\Program Files\Subversion\bin"
svn co http://10.0.0.170/pluto/trunk/src /pluto.makerelease/trunk/src
svn co http://10.0.0.170/pluto/trunk/libs /pluto.makerelease/trunk/libs
svn co http://10.0.0.170/pluto/trunk/installers/Installers /pluto.makerelease/trunk/installers/Installers

echo Building pluto(symbian) sources...
cd "c:\pluto.makerelease\trunk\src"
call "MakeRelease_Symbian_S60.bat"

echo Building pluto(windows) sources...
cd "c:\pluto.makerelease\trunk\src"
call "MakeRelease_Windows.bat"

echo Building pluto(windows ce) sources...
cd "c:\pluto.makerelease\trunk\src"
call "MakeRelease_WindowsCE.bat"

cd "c:\pluto.makerelease\trunk\src\bin"
copy Orbiter.exe Orbiter_Win32.dat
copy Orbiter_CeNet4_XScale.exe Orbiter_CeNet4_XScale.dat
copy OrbiterSmartphone.exe OrbiterSmartphone.dat

echo Copying EXE files...
copy *.exe "\\10.0.0.150\builds\Windows_Output\src\bin"
echo Copying DLL files...
copy *.dll "\\10.0.0.150\builds\Windows_Output\src\bin"
echo Copying DAT files...
copy *.dat "\\10.0.0.150\builds\Windows_Output\src\bin"
echo Copying SIS files...
copy *.sis "\\10.0.0.150\builds\Windows_Output\src\bin"
echo Copying CAB files...
copy *.cab "\\10.0.0.150\builds\Windows_Output\src\bin"

echo Copying LIB files...
cd "c:\pluto.makerelease\trunk\src\lib"
copy *.lib "\\10.0.0.150\builds\Windows_Output\src\lib"

echo Copying MSI files...
cd C:\pluto.makerelease\trunk\installers\Installers\Orbiter\Release\
copy *.msi "\\10.0.0.150\builds\Windows_Output\src\bin"


