@ECHO OFF
REM #########################################################
REM 
REM \brief  Create binary distribution (without installer).
REM
REM         Creates binary distribution - without installer.
REM         This may be useful for those wishing to incorporate
REM         myodbc into their own installer.
REM 
REM \note   To do this you are going to need pkzipc (the command-line
REM         version of pkzip) somewhere in your path.
REM
REM \sa     CreateBinaryMsi.bat
REM         CreateSourceZip.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax

ECHO Building...
call Build.bat

ECHO Clean any existing stage area...
IF EXIST .\mysql-connector-odbc-noinstall-%1-win32.zip ( 
    del mysql-connector-odbc-noinstall-%1-win32.zip 
)
IF EXIST .\mysql-connector-odbc-noinstall-%1-win32 ( 
    rmdir /S /Q mysql-connector-odbc-noinstall-%1-win32 
)

IF EXIST .\mysql-connector-odbc-commercial-noinstall-%1-win32.zip ( 
    del mysql-connector-odbc-commercial-noinstall-%1-win32.zip 
)
IF EXIST .\mysql-connector-odbc-commercial-noinstall-%1-win32 ( 
    rmdir /S /Q mysql-connector-odbc-commercial-noinstall-%1-win32 
)

ECHO GPL: Create stage area and populate...
mkdir mysql-connector-odbc-noinstall-%1-win32 
mkdir mysql-connector-odbc-noinstall-%1-win32\bin
mkdir mysql-connector-odbc-noinstall-%1-win32\lib
mkdir mysql-connector-odbc-noinstall-%1-win32\doc
xcopy /E /Y bin\* mysql-connector-odbc-noinstall-%1-win32\bin
xcopy /E /Y lib\* mysql-connector-odbc-noinstall-%1-win32\lib
xcopy /E /Y doc\* mysql-connector-odbc-noinstall-%1-win32\doc
copy Install.bat mysql-connector-odbc-noinstall-%1-win32
copy Uninstall.bat mysql-connector-odbc-noinstall-%1-win32
copy ChangeLog mysql-connector-odbc-noinstall-%1-win32\ChangeLog.rtf
copy LICENSE.gpl mysql-connector-odbc-noinstall-%1-win32\LICENSE.rtf
copy LICENSE.exceptions mysql-connector-odbc-noinstall-%1-win32\EXCEPTIONS.rtf
copy README mysql-connector-odbc-noinstall-%1-win32\README.rtf
copy INSTALL mysql-connector-odbc-noinstall-%1-win32\INSTALL.rtf
copy INSTALL.win mysql-connector-odbc-noinstall-%1-win32\INSTALL-win.rtf

ECHO Zipping...
pkzipc -add -maximum -recurse -path=current mysql-connector-odbc-noinstall-%1-win32.zip mysql-connector-odbc-noinstall-%1-win32\*.*

ECHO COMMERCIAL: Create stage area and populate...
move mysql-connector-odbc-noinstall-%1-win32 mysql-connector-odbc-commercial-noinstall-%1-win32 
copy LICENSE.commercial mysql-connector-odbc-commercial-noinstall-%1-win32\LICENSE.rtf
del mysql-connector-odbc-commercial-noinstall-%1-win32\EXCEPTIONS.rtf

ECHO Zipping...
pkzipc -add -maximum -recurse -path=current mysql-connector-odbc-commercial-noinstall-%1-win32.zip mysql-connector-odbc-commercial-noinstall-%1-win32/*.*
IF EXIST .\mysql-connector-odbc-commercial-noinstall-%1-win32 ( 
    rmdir /S /Q mysql-connector-odbc-commercial-noinstall-%1-win32 
)

EXIT /B 0

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| CreateBinaryZip.bat                                 |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to build sources and create a ZIP based    |"
ECHO "| noinstaller distribution for commercial and GPL.    |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| CreateBinaryZip <version>                           |"
ECHO "|                                                     |"
ECHO "| <version>   must be a 3 number version              |"
ECHO "|                                                     |"
ECHO "| EXAMPLE                                             |"
ECHO "|                                                     |"
ECHO "| CreateBinaryZip 3.51.12                             |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"

