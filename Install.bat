@ECHO OFF
REM #########################################################
REM 
REM \brief  Install myodbc.
REM
REM         This exists for those working with the Windows source
REM         distribution.
REM
REM         Use this to copy the driver and supporting files
REM         to the system directory and register the driver.
REM
REM \sa     README.win
REM
REM #########################################################

SET installdir=none
IF EXIST %windir%\system\nul   SET installdir=%windir%\system
IF EXIST %windir%\system32\nul SET installdir=%windir%\system32
IF %installdir%==none GOTO :doError5

IF EXIST %installdir%\myodbc-installer.exe GOTO :doError4

REM ****
REM * Find out the bin/lib directory, or use default
REM ****
SET libdir=lib
SET bindir=bin
IF EXIST lib\release\myodbc5.lib         SET libdir=lib\release
IF EXIST lib\relwithdebinfo\myodbc5.lib  SET libdir=lib\relwithdebinfo
IF EXIST bin\release\myodbc-installer.exe        SET bindir=bin\release
IF EXIST bin\relwithdebinfo\myodbc-installer.exe SET bindir=bin\relwithdebinfo

REM ****
REM * Copying myodbc libraries and executables to install dir...
REM ****
ECHO Copying installation files
IF NOT EXIST %libdir%\myodbc5.lib  GOTO :doError2
IF NOT EXIST %libdir%\myodbc5S.lib GOTO :doError2
IF NOT EXIST %bindir%\myodbc-installer.exe GOTO :doError2
copy %libdir%\myodbc5S.dll %installdir%
copy %libdir%\myodbc5S.lib %installdir%
copy %libdir%\myodbc5.dll  %installdir%
copy %libdir%\myodbc5.lib  %installdir%
copy %bindir%\myodbc-installer.exe      %installdir%

REM ****
REM * Registering driver...
REM *
REM * We can do this with myodbc-installer.exe or the MS Windows ODBCConf.exe. It
REM * may be safer to use the ODBCConf.exe when we think about such things
REM * as 64bit windows. 
REM ****
ECHO Registering driver
myodbc-installer -d -a -n "MySQL ODBC 5.1 Driver" -t "DRIVER=myodbc5.dll;SETUP=myodbc5S.dll"

ECHO "+-----------------------------------------------------+"
ECHO "| DONE                                                |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Hopefully things went well; the Connector/ODBC      |"
ECHO "| files have been copied to the system directory      |"
ECHO "| and the driver has been registered.                 |"
ECHO "|                                                     |"
ECHO "| Connector/ODBC is ready to use.                     |"
ECHO "|                                                     |"
ECHO "| The most common thing to do next is to go to the    |"
ECHO "| Control Panel and find the ODBC Administrator -     |"
ECHO "| then use it to create a Data Source Name (DSN)      |"
ECHO "| so you (and your application) can connect to a      |"
ECHO "| MySQL server.                                       |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
EXIT /B 0

:doError2
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Connector/ODBC not built.                           |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

:doError4
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Existing Connector/ODBC installed. Request ignored. |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

:doError5
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Can't find the Windows system directory             |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

