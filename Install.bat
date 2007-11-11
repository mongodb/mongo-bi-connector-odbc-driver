@ECHO OFF
REM #########################################################
REM 
REM \brief  Install myodbc.
REM
REM         This exists for those working with the Windows source
REM         distribution.
REM
REM \sa     README.win
REM
REM #########################################################

SET installdir=none
IF EXIST %windir%\system\nul   SET installdir=%windir%\system
IF EXIST %windir%\system32\nul SET installdir=%windir%\system32
IF %installdir%==none GOTO :doError5

IF "%1"=="1" GOTO :doDebug
IF "%1"=="0" GOTO :doNormal
GOTO doSyntax

:doNormal
IF EXIST %installdir%\myodbc3i.exe GOTO :doError4

REM ****
REM * Find out the bin/lib directory, or use default
REM ****
SET libdir=lib
SET bindir=bin
IF EXIST lib\release\myodbc5.lib         SET libdir=lib\release
IF EXIST lib\relwithdebinfo\myodbc5.lib  SET libdir=lib\relwithdebinfo
IF EXIST bin\release\myodbc3i.exe        SET bindir=bin\release
IF EXIST bin\relwithdebinfo\myodbc3i.exe SET bindir=bin\relwithdebinfo

REM ****
REM * Copying myodbc libraries and executables to install dir...
REM ****
IF NOT EXIST %libdir%\myodbc5.lib  GOTO :doError2
IF NOT EXIST %libdir%\myodbc5S.lib GOTO :doError2
IF NOT EXIST %bindir%\myodbc3i.exe GOTO :doError2
IF NOT EXIST %bindir%\myodbc3m.exe GOTO :doError2
copy %libdir%\myodbc5S.dll %installdir%
copy %libdir%\myodbc5S.lib %installdir%
copy %libdir%\myodbc5.dll  %installdir%
copy %libdir%\myodbc5.lib  %installdir%
copy %bindir%\myodbc3i.exe      %installdir%
copy %bindir%\myodbc3m.exe      %installdir%
copy doc\*.hlp             %installdir%

REM ****
REM * Registering driver...
REM *
REM * We can do this with myodbc3i.exe or the MS Windows ODBCConf.exe. It
REM * may be safer to use the ODBCConf.exe when we think about such things
REM * as 64bit windows. 
REM ****
myodbc3i -a -d -t"MySQL ODBC 5.1 Driver;DRIVER=myodbc5.dll;SETUP=myodbc5S.dll"

GOTO doSuccess


:doDebug
REM ****
REM * Find out the bin/lib directory, or use default
REM ****
SET libdir=lib
IF EXIST lib\debug\myodbc5d.lib          SET libdir=lib\debug

IF NOT EXIST %libdir%\myodbc5d.lib goto doError3
IF NOT EXIST %libdir%\myodbc5E.lib goto doError3
IF NOT EXIST %installdir%\myodbc3i.exe goto doError1
REM ****
REM * Copying myodbc debug libraries to install dir...
REM ****
copy %libdir%\myodbc5E.dll %installdir%
copy %libdir%\myodbc5E.lib %installdir%
copy %libdir%\myodbc5d.dll %installdir%
copy %libdir%\myodbc5d.lib %installdir%

REM ****
REM * Registering driver...
REM ****
myodbc3i -a -d -t"MySQL ODBC 5.1 Driver (debug);DRIVER=myodbc5d.dll;SETUP=myodbc5E.dll"

goto doSuccess


:doSuccess
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

:doError1
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| The non-debug version of Connector/ODBC needs to be |"
ECHO "| installed.                                          |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

:doError2
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Connector/ODBC not built. Consider executing        |"
ECHO "| Build.bat.                                          |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

:doError3
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Connector/ODBC (debug) not built. Consider executing|"
ECHO "| Build.bat.                                          |"
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

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| Install.bat                                         |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to copy the driver and supporting files    |"
ECHO "| to the system directory and register the driver.    |"
ECHO "|                                                     |"
ECHO "| You can not properly install the debug version      |"
ECHO "| without first installing the regular version.       |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| Install <debug>                                     |"
ECHO "|                                                     |"
ECHO "| <debug>  must be;                                   |"
ECHO "|              0 - to install a regular build         |"
ECHO "|              1 - to install a debug version         |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"

