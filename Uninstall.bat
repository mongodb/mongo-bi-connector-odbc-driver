@ECHO OFF
REM #########################################################
REM
REM \brief  Uninstall myodbc.
REM
REM         This exists for those working with the Windows source
REM         distribution.
REM
REM \sa     README.win
REM
REM #########################################################

IF "%1"=="0" GOTO doNormal
IF "%1"=="1" GOTO doDebug
GOTO doSyntax

:doNormal
IF NOT EXIST \Windows\System32\myodbc3i.exe GOTO doError2
REM ****
REM * Deregistering driver...
REM ****
myodbc3i -r -d -n"MySQL ODBC 3.51 Driver"

REM ****
REM * Removing files...
REM ****
del /Q /F \Windows\System32\myodbc3S.dll
del /Q /F \Windows\System32\myodbc3S.lib
del /Q /F \Windows\System32\myodbc3.dll
del /Q /F \Windows\System32\myodbc3.lib
del /Q /F \Windows\System32\myodbc3i.exe
del /Q /F \Windows\System32\myodbc3m.exe
del /Q /F \Windows\System32\myodbc3c.exe
del /Q /F \Windows\System32\myodbc3*.hlp
GOTO doSuccess

:doDebug
IF NOT EXIST \Windows\System32\myodbc3d.dll GOTO doError3
IF NOT EXIST \Windows\System32\myodbc3i.exe GOTO doError1
REM ****
REM * Deregistering driver...
REM ****
myodbc3i -r -d -n"MySQL ODBC 3.51 Driver (debug)"

REM ****
REM * Removing files...
REM ****
del /Q /F \Windows\System32\myodbc3E.dll
del /Q /F \Windows\System32\myodbc3E.lib
del /Q /F \Windows\System32\myodbc3d.dll
del /Q /F \Windows\System32\myodbc3d.lib
GOTO doSuccess

:doSuccess
ECHO "+-----------------------------------------------------+"
ECHO "| DONE                                                |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Hopefully things went well; the Connector/ODBC      |"
ECHO "| files have been removed from the system directory   |"
ECHO "| and the driver has been deregistered.               |"
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
ECHO "| Connector/ODBC does not appear to be installed.     |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

:doError3
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Connector/ODBC (debug) does not appear to be        |"
ECHO "| installed.                                          |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| Uninstall.bat                                       |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to remove the driver and supporting files  |"
ECHO "| from the system directory and deregister the driver.|"
ECHO "|                                                     |"
ECHO "| The regular version must be installed for the       |"
ECHO "| debug version to be properly uninstalled.           |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| Uninstall <debug>                                   |"
ECHO "|                                                     |"
ECHO "| <debug>  must be;                                   |"
ECHO "|              0 - to uninstall a regular build       |"
ECHO "|              1 - to uninstall a debug version       |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"

