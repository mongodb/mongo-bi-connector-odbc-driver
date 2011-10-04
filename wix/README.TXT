MSI build instructions

1) Prerequisites

- Wix 3.0 or above (http://wix.codeplex.com/)
- Resources from connectors-svnroot/wix-installer
- Microsoft Visual Studio environment
- CMake 2.4.6 (http://www.cmake.org)

2) Environment variables

- WIX_DIR
If Wix executables are not in your default path, you need to specify
WIX_DIR environment variable which points to your Wix installation 
directory.

- WIX_RESOURCE
Resource directory from svn+ssh://<your-uid>@bk-internal.mysql.com/connectors-svnroot/wix-installer

3) Copying files

Create the directors "doc" (for license files), and "x86"
(for 32-bit binaries). To create a 64-bit package you create a "x64"
bit directory as well, i.e. a 64-bit package contains both 32-bit and
64-bit binaries. Copy the required files:

  doc/README.txt
  doc/COPYING.txt
  doc/Licenses_for_Third-Party_Components.txt
  x86/myodbc5.lib
  x86/myodbc5.dll
  x86/myodbc5S.lib
  x86/myodbc5S.dll
  x86/myodbc-installer.exe

and in addition if a 64-bit package

  x64/myodbc5.lib
  x64/myodbc5.dll
  x64/myodbc5S.lib
  x64/myodbc5S.dll
  x64/myodbc-installer.exe

4) CMake

There are two CMake options you can use to control the build

  -DMSI_64=1   build a 64-bit package
  -DPDB=1      include .pdb files

In the simple 32-bit case you just run

  cmake -G "NMake Makefiles"

5) Build MSI

Start nmake for building MyODBC msi file
