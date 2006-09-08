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

IF "%1"=="1" GOTO :doDebug
IF "%1"=="0" GOTO :doNormal
GOTO doSyntax

:doNormal
IF EXIST \Windows\System32\myodbc3i.exe GOTO :doError4
IF NOT EXIST lib\myodbc3.lib GOTO :doError2
IF NOT EXIST lib\myodbc3S.lib GOTO :doError2
REM ****
REM * Copying myodbc libraries and executables to install dir...
REM ****
copy lib\qt-mt335.dll \Windows\System32
copy lib\myodbc3S.dll \Windows\System32
copy lib\myodbc3S.lib \Windows\System32
copy lib\myodbc3.dll \Windows\System32
copy lib\myodbc3.lib \Windows\System32
copy bin\myodbc3i.exe \Windows\System32
copy bin\myodbc3m.exe \Windows\System32
copy bin\myodbc3c.exe \Windows\System32
copy doc\*.hlp \Windows\System32

REM ****
REM * Registering driver...
REM *
REM * We can do this with myodbc3i.exe or the MS Windows ODBCConf.exe. It
REM * may be safer to use the ODBCConf.exe when we think about such things
REM * as 64bit windows. 
REM ****
myodbc3i -a -d -t"MySQL ODBC 3.51 Driver;DRIVER=myodbc3.dll;SETUP=myodbc3S.dll"

GOTO doSuccess


:doDebug
IF NOT EXIST lib\myodbc3d.lib goto doError3
IF NOT EXIST lib\myodbc3E.lib goto doError3
IF NOT EXIST \Windows\System32\myodbc3i.exe goto doError1
REM ****
REM * Copying myodbc debug libraries to install dir...
REM ****
copy lib\myodbc3E.dll \Windows\System32
copy lib\myodbc3E.lib \Windows\System32
copy lib\myodbc3d.dll \Windows\System32
copy lib\myodbc3d.lib \Windows\System32

REM ****
REM * Registering driver...
REM ****
myodbc3i -a -d -t"MySQL ODBC 3.51 Driver (debug);DRIVER=myodbc3d.dll;SETUP=myodbc3E.dll"

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

