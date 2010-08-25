@ECHO OFF
REM #########################################################
REM
REM \brief  Uninstall myodbc.
REM
REM         This exists for those working with the Windows source
REM         distribution.
REM
REM         Use this to remove the driver and supporting files
REM         from the system directory and deregister the driver.
REM
REM \sa     README.win
REM
REM #########################################################

SET installdir=none
IF EXIST %windir%\system\nul   SET installdir=%windir%\system
IF EXIST %windir%\system32\nul SET installdir=%windir%\system32
IF %installdir%==none GOTO :doError4

IF NOT EXIST %installdir%\myodbc-installer.exe GOTO doError2
REM ****
REM * Deregistering driver...
REM ****
myodbc-installer -d -r -n "MySQL ODBC 5.1 Driver"

REM ****
REM * Removing files...
REM ****
del /Q /F %installdir%\myodbc5S.dll
del /Q /F %installdir%\myodbc5S.lib
del /Q /F %installdir%\myodbc5.dll
del /Q /F %installdir%\myodbc5.lib
del /Q /F %installdir%\myodbc-installer.exe

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

:doError4
ECHO "+-----------------------------------------------------+"
ECHO "| ERROR                                               |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Can't find the Windows system directory             |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
PAUSE
EXIT /B 1

