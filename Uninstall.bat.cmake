@ECHO OFF
REM #########################################################
REM
REM \brief  Deregister Connector/ODBC driver registered with
REM         Install.bat
REM
REM         This exists for those working with the Windows 
REM         source distribution or with installer-less 
REM         binary distribution.
REM
REM         If driver was registerd under non-default name,
REM         the name used should be specified as first 
REM         parameter of this script.
REM
REM         Note that you should manually remove all data
REM         sources using a driver before uninstalling that
REM         driver.
REM
REM \sa     README.win
REM
REM #########################################################

REM # SETLOCAL prevents the variables set in this script to
REM # be exported to the environment and pollute it
SETLOCAL

SET    driver_name=MySQL ODBC @CONNECTOR_MAJOR@.@CONNECTOR_MINOR@(@CONNECTOR_DRIVER_TYPE_SHORT@) Driver
SET      installer=myodbc-installer
SET   driver_found=no

IF "%1" == "" GOTO :doFindInstaller
SET  driver_name=%1

:doFindInstaller
REM # Find the installer utility

SET bindir=none
FOR %%G IN (. bin bin\release bin\relwithdebinfo bin\debug) DO CALL :subFindInstaller %%G

SET myodbc_installer=%bindir%\%installer%.exe
IF NOT "%bindir%" == "none" GOTO :doDeregister

REM # Try if it is in the path
SET myodbc_installer=%installer%.exe
"%myodbc_installer%" >nul 2>nul
REM # "Command not found" generates error 9009
IF NOT ERRORLEVEL 9000 GOTO :doDeregister

GOTO :errorNoInstaller

REM ######
REM # A subroutine to check if given location
REM # (relative to working dir) contains myodbc
REM # installer utility.
REM ######
:subFindInstaller
REM # Skip check if a good libdir was already found
IF NOT "%bindir%" == "none" GOTO :eof
SET bindir=%CD%\%1
IF NOT EXIST "%bindir%\%installer%.exe"  GOTO :wrongBinDir
REM ECHO Bindir (%bindir%) is OK.
GOTO :eof
:wrongBinDir
REM ECHO Bindir (%bindir%) is wrong.
SET bindir=none
GOTO :eof

:doDeregister
ECHO "installer = %myodbc_installer%"

REM # Check if driver is registered

"%myodbc_installer%" -d -l -n "%driver_name%" 2>nul
IF ERRORLEVEL 1 GOTO :errorNotRegistered
SET driver_found=yes

ECHO Deregistering %driver_name%
"%myodbc_installer%" -d -r -n "%driver_name%"

:doSuccess
ECHO ^+-----------------------------------------------------^+
ECHO ^| DONE                                                ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Hopefully things went well; the Connector/ODBC      ^|
ECHO ^| driver has been deregistered.                       ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 0

:errorNoInstaller
ECHO ^+-----------------------------------------------------^+
ECHO ^| ERROR                                               ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Could not find the MyODBC Installer utility. Run    ^|
ECHO ^| this script from the installation directory.        ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 1

:errorNotRegistered
ECHO ^+-----------------------------------------------------^+
ECHO ^| ERROR                                               ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Connector/ODBC does not appear to be registered.    ^|
ECHO ^| Was it registered under non-default name which      ^|
ECHO ^| then should be specified as the first parameter of  ^|
ECHO ^| this script?                                        ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 1
