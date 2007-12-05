MSI build instructions

1) Prerequisites

- Wix 3.0 or above (http://wix.sourceforge.net)
- Resources from connectors-svnroot/wix-installer
- Microsoft Visual Studio environment
- CMake 2.4.6 (http://www.cmake.org)

2) Environment variables

- WIX_DIR
If Wix executables are not in your default path, you need to specify
WIX_DIR environment variable which points to your Wix installation 
directory.

- WIX_RESOURCES
Resource directory from connectors-svnroot/wix-installer

3) Copying files

Create the directories "doc" (for chm and license files),
"x86" (for 32-bit binaries) and "x64" for 64-bit binaries and
copy the required files:

doc/connector-odbc.chm
doc/MYODBC5-LICENSE.TXT
doc/MYODBC5-EXCEPTIONS.TXT
x86/myodbc5.lib
x86/myodbc5.dll
x86/myodbc5S.lib
x86/myodbc5S.dll
x86/myodbc-installer.exe
x64/myodbc5.lib
x64/myodbc5.dll
x64/myodbc5S.lib
x64/myodbc5S.dll
x64/myodbc-installer.exe

4) Cmake

Start CMake (for 64-bit packages you need to define -DMSI_64=1)

cmake -G "NMake Makefiles"

5) Build MSI

start nmake for building MyODBC msi file
