ECHO OFF
REM #########################################################
REM 
REM \brief  Create Makefiles from qmake project files.
REM
REM         Actually; you only want to use this to 'regenerate'
REM         the Makefiles. So do a qmake from the root source for
REM         the first time and then after that you probably want
REM         to use this for regenerating them due to changes in 
REM         a qmake project file.
REM
REM         It would appear that under certian circumtsances existing
REM         make files will not get regenerated after a qmake project
REM         file has changed.
REM 
REM \note   Its probably a good idea to run this twice as I think the
REM         the problem only occurs the first time an attempt is made
REM         to auto recreate the makefile. 
REM 
REM #########################################################

ECHO ****
ECHO * Recreating Makefiles...
ECHO ****
qmake

ECHO MYODBCDbg...
cd MYODBCDbg\MYODBCDbgLib
qmake
cd ..
qmake
cd ..

ECHO dltest...
cd dltest
qmake
cd ..

ECHO myodbc3u...
cd myodbc3u
qmake
cd ..

ECHO myodbc3S...
cd myodbc3S
qmake
cd ..

ECHO myodbc3i...
cd myodbc3i
qmake
cd ..

ECHO myodbc3m...
cd myodbc3m
qmake
cd ..

ECHO myodbc3c...
cd myodbc3c
qmake
cd ..

ECHO myodbc3...
cd myodbc3
qmake
cd ..

ECHO MYODBCCls...
cd MYODBCCls\MYODBCClsLib
qmake
cd ..\MYODBCClsApp
qmake
cd ..
qmake
cd ..

ECHO MYQTODBCCls...
cd MYQTODBCCls\MYQTODBCClsLib
qmake
cd ..\MYQTODBCClsApp
qmake
cd ..
qmake
cd ..

ECHO MYODBCBrw...
cd MYODBCBrw\MYODBCBrwLib
qmake
cd ..\MYODBCBrwApp
qmake
cd ..
qmake
cd ..

ECHO ****
ECHO * You should now have fresh makefiles to work with. Any changes to
ECHO * qmake project files should be reflected in these new makefiles.
ECHO ****


