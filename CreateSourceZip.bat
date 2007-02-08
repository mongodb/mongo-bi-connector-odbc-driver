@ECHO OFF
REM #########################################################
REM 
REM \brief  Create source distribution.
REM
REM         Creates source distribution for windows.
REM 
REM \note   To do this you are going to need pkzipc (the command-line
REM         version of pkzip) somewhere in your path.
REM
REM \sa     CreateBinaryMsi.bat
REM         CreateBinaryZip.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax

ECHO Clean any existing stage area...
IF EXIST mysql-connector-odbc-%1-win-src.zip ( 
    del mysql-connector-odbc-%1-win-src.zip 
)
IF EXIST mysql-connector-odbc-%1-win-src ( 
    rmdir /S /Q mysql-connector-odbc-%1-win-src 
)

ECHO Create stage area and populate...
mkdir mysql-connector-odbc-%1-win-src 

REM Its easier to copy specific files then try 
REM to clean out garbage (in this case).
REM xcopy /E /Y * mysql-connector-odbc-%1-win-src

REM root
mkdir mysql-connector-odbc-%1-win-src 
copy ChangeLog mysql-connector-odbc-%1-win-src\ChangeLog.rtf
copy LICENSE.gpl mysql-connector-odbc-%1-win-src\LICENSE.rtf
copy LICENSE.exceptions mysql-connector-odbc-%1-win-src\EXCEPTIONS.rtf
copy Build.bat mysql-connector-odbc-%1-win-src
copy Install.bat mysql-connector-odbc-%1-win-src
copy Uninstall.bat mysql-connector-odbc-%1-win-src
copy CreateMakefiles.bat mysql-connector-odbc-%1-win-src
copy CreateVisualStudioProjects.bat mysql-connector-odbc-%1-win-src
copy config.pri mysql-connector-odbc-%1-win-src
copy defines.pri mysql-connector-odbc-%1-win-src
copy mysql.pri mysql-connector-odbc-%1-win-src
copy odbc.pri mysql-connector-odbc-%1-win-src
copy root.pro mysql-connector-odbc-%1-win-src
copy README mysql-connector-odbc-%1-win-src\README.rtf
copy BUILD mysql-connector-odbc-%1-win-src\BUILD.rtf
copy BUILD.win mysql-connector-odbc-%1-win-src\BUILD-win.rtf
copy INSTALL mysql-connector-odbc-%1-win-src\INSTALL.rtf
copy INSTALL.win mysql-connector-odbc-%1-win-src\INSTALL-win.rtf
copy resource.h mysql-connector-odbc-%1-win-src
copy VersionInfo.h mysql-connector-odbc-%1-win-src
copy mysql.bmp mysql-connector-odbc-%1-win-src

REM dltest
mkdir mysql-connector-odbc-%1-win-src\dltest
copy dltest\*.pro mysql-connector-odbc-%1-win-src\dltest
copy dltest\*.c mysql-connector-odbc-%1-win-src\dltest

REM util
mkdir mysql-connector-odbc-%1-win-src\util
copy util\*.pro mysql-connector-odbc-%1-win-src\util
copy util\*.h mysql-connector-odbc-%1-win-src\util
copy util\*.c mysql-connector-odbc-%1-win-src\util

REM setup
mkdir mysql-connector-odbc-%1-win-src\setup
copy setup\*.pro mysql-connector-odbc-%1-win-src\setup
copy setup\*.def mysql-connector-odbc-%1-win-src\setup
copy setup\*.xpm mysql-connector-odbc-%1-win-src\setup
copy setup\*.rc mysql-connector-odbc-%1-win-src\setup
copy setup\*.c mysql-connector-odbc-%1-win-src\setup
copy setup\*.cpp mysql-connector-odbc-%1-win-src\setup
copy setup\*.h mysql-connector-odbc-%1-win-src\setup

REM installer
mkdir mysql-connector-odbc-%1-win-src\installer
copy installer\*.pro mysql-connector-odbc-%1-win-src\installer
copy installer\*.c mysql-connector-odbc-%1-win-src\installer

REM monitor
mkdir mysql-connector-odbc-%1-win-src\monitor 
copy monitor\*.pro mysql-connector-odbc-%1-win-src\monitor
copy monitor\*.c mysql-connector-odbc-%1-win-src\monitor
copy monitor\*.h mysql-connector-odbc-%1-win-src\monitor

REM dsn-editor
mkdir mysql-connector-odbc-%1-win-src\dsn-editor 
copy dsn-editor\*.pro mysql-connector-odbc-%1-win-src\dsn-editor
copy dsn-editor\*.cpp mysql-connector-odbc-%1-win-src\dsn-editor

REM driver
mkdir mysql-connector-odbc-%1-win-src\driver
copy driver\*.pro mysql-connector-odbc-%1-win-src\driver
copy driver\*.c mysql-connector-odbc-%1-win-src\driver
copy driver\*.h mysql-connector-odbc-%1-win-src\driver
copy driver\*.rc mysql-connector-odbc-%1-win-src\driver
copy driver\*.def mysql-connector-odbc-%1-win-src\driver

REM doc
mkdir mysql-connector-odbc-%1-win-src\doc
copy doc\* mysql-connector-odbc-%1-win-src\doc

ECHO Zipping...
pkzipc -add -maximum -recurse -path=current mysql-connector-odbc-%1-win-src.zip mysql-connector-odbc-%1-win-src/*.*
IF EXIST mysql-connector-odbc-%1-win-src ( 
    rmdir /S /Q mysql-connector-odbc-%1-win-src
)

ECHO ****
ECHO * Hopefully things went well and you have a fresh new source distribution
ECHO * in the source root directory. 
ECHO ****

EXIT /B 0

:doSyntax
ECHO CreateSourceZip
ECHO ===============
ECHO .
ECHO Usage:
ECHO .
ECHO %0 VERSION
ECHO .
ECHO   VERSION      must be; a 3 number version code
ECHO .
ECHO   Examples:
ECHO .
ECHO   %0 3.51.12

