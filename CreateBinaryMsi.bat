@ECHO OFF
REM #########################################################
REM 
REM \brief  Create binary distribution (with installer).
REM
REM         Creates binary distributions with installer. It will
REM         create a GPL and a Commercial version.
REM         1) MSI
REM         2) setup.exe (zipped)
REM 
REM \sa     CreateBinaryZip.bat
REM         CreateSourceZip.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax
IF "%2"=="" GOTO doSyntax

REM Building...
call Build.bat

REM Copying files to wix stage area...
IF NOT EXIST ..\wix-installer\bin\mysql-connector-odbc-%1-win32 (
    mkdir ..\wix-installer\bin\mysql-connector-odbc-%1-win32
)
IF NOT EXIST ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows (
    mkdir ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows
)
IF NOT EXIST ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32 (
    mkdir ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
)
copy bin\* ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy lib\* ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy doc\*.hlp ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy LICENSE.* ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32

REM Creating Commercial msi...
cd ..\wix-installer
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\LICENSE.commercial bin\mysql-connector-odbc-%1-win32\Windows\System32\myodbc3-license.rtf
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\LICENSE.commercial resources\commercial_license.rtf
call OdbcMakeSetup.bat %1 %2 commercial

REM Creating GPL msi...
move /Y bin\dist\mysql-connector-odbc-%1-win32.msi bin\dist\mysql-connector-odbc-commercial-%1-win32.msi
move /Y bin\dist\mysql-connector-odbc-%1-win32.msi bin\dist\mysql-connector-odbc-commercial-%1-win32.msi
move /Y bin\dist\mysql-connector-odbc-%1-win32.zip bin\dist\mysql-connector-odbc-commercial-%1-win32.zip
move /Y bin\dist\mysql-connector-odbc-%1-win32.msi.md5 bin\dist\mysql-connector-odbc-commercial-%1-win32.msi.md5
move /Y bin\dist\mysql-connector-odbc-%1-win32.zip.md5 bin\dist\mysql-connector-odbc-commercial-%1-win32.zip.md5
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\LICENSE.gpl bin\mysql-connector-odbc-%1-win32\Windows\System32\myodbc3-license.rtf
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\LICENSE.exceptions bin\mysql-connector-odbc-%1-win32\Windows\System32\myodbc3-exceptions.rtf
call OdbcMakeSetup.bat %1 %2 gpl

cd ..\*odbc3

EXIT /B 0

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| CreateBinaryMsi.bat                                 |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to build sources and create a MSI based    |"
ECHO "| installers for commercial and GPL.                  |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| CreateBinaryMsi <version> <build-type>              |"
ECHO "|                                                     |"
ECHO "| <version>   must be a 3 number version              |"
ECHO "|                                                     |"
ECHO "| <built-type> must be;                               |"
ECHO "|              a - alpha                              |"
ECHO "|              b - beta                               |"
ECHO "|              r - release candidate                  |"
ECHO "|              p - production                         |"
ECHO "|              i - internal                           |"
ECHO "|                                                     |"
ECHO "| EXAMPLE                                             |"
ECHO "|                                                     |"
ECHO "| CreateBinaryMsi 3.51.12 p                           |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"

