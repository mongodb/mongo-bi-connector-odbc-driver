@ECHO OFF
REM #########################################################
REM 
REM \brief  Create Visual Studio project files.
REM
REM         The Visual Studio project files may already exist
REM         but this can be used to create/recreate them. The Visual 
REM         Studio project files are created using qmake and 
REM         the qmake project files. So if the qmake project files
REM         change for some reason - it may be a good idea to use
REM         this to recreate the corresponding Visual Studio 
REM         project files.
REM 
REM         A Visual Studio 'Solution' containing all the projects
REM         may be found in sources.
REM 
REM \note   The command-line is the preferred method of building 
REM         on all platforms - including MS Windows.
REM 
REM \sa     root.pro
REM
REM #########################################################

ECHO Creating Visual Studio project files...

ECHO MYODBCDbg...
cd MYODBCDbg\MYODBCDbgLib
qmake -t vclib
cd ..
qmake
cd ..

ECHO dltest...
cd dltest
qmake -t vcapp
cd ..

ECHO myodbc3u...
cd myodbc3u
qmake -t vclib
cd ..

ECHO myodbc3S...
cd myodbc3S
qmake -t vclib
cd ..

ECHO installer...
cd installer
qmake -t vcapp
cd ..

ECHO myodbc3m...
cd myodbc3m
qmake -t vcapp
cd ..

ECHO myodbc3c...
cd myodbc3c
qmake -t vcapp
cd ..

ECHO myodbc3...
cd myodbc3
qmake -t vclib
cd ..
