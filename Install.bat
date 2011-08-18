@ECHO OFF
REM #########################################################
REM 
REM \brief  Register Connector/ODBC driver with ODBC system
REM   
REM         This exists for those working with the Windows 
REM         source distribution or with installer-less 
REM         binary distribution.
REM
REM         Name under which the driver should be registered
REM         can be specified as first parameter. It should be
REM         a single word (no spaces).
REM
REM \sa     README.win
REM
REM #########################################################

REM # SETLOCAL prevents the variables set in this script to
REM # be exported to the environment and pollute it
SETLOCAL

SET         driver_name=MySQL ODBC 5.1 Driver
SET          driver_lib=myodbc5
SET    driver_lib_setup=myodbc5S
SET           installer=myodbc-installer

IF "%1" == "" GOTO :doFindDriver
SET  driver_name=%1

:doFindDriver
REM # Find driver location

SET libdir=none
FOR %%G IN (. lib lib\release lib\relwithdebinfo lib\debug) DO CALL :subCheckLibDir %%G

IF "%libdir%" == "none" GOTO :errorNoDrivers
REM ECHO "libdir = %libdir%"

REM # Find the installer utility

REM # Try to find it in the build location
CALL :subFindBinDir "%libdir%"
SET myodbc_installer=%bindir%\%installer%.exe
IF EXIST "%myodbc_installer%" GOTO :doRegister

REM # Try some other reasonable locations
SET myodbc_installer=bin\%installer%.exe
IF EXIST "%myodbc_installer%" GOTO :doRegister
SET myodbc_installer=.\%installer%.exe
IF EXIST "%myodbc_installer%" GOTO :doRegister

REM # Try if it is in the path
SET myodbc_installer=%installer%.exe
%myodbc_installer% >nul 2>nul
REM # "Command not found" generates error 9009
IF NOT ERRORLEVEL 9000 GOTO :doRegister

GOTO :errorNoInstaller

:doRegister
REM ECHO myodbc_installer: %myodbc_installer%

REM # Abort if driver is already registered

%myodbc_installer% -d -l -n "%driver_name%" 2>nul:
IF NOT ERRORLEVEL 1 GOTO :errorDriverInstalled

ECHO Registering %driver_name%
%myodbc_installer% -d -a -n "%driver_name%" -t "DRIVER=%libdir%\%driver_lib%.dll;SETUP=%libdir%\%driver_lib_setup%.dll"

IF ERRORLEVEL 1 GOTO :errorRegisterDriver

GOTO :doSuccess

REM ######
REM # A subroutine to check if given location
REM # (relative to working dir) contains the drivers.
REM ######
:subCheckLibDir
REM # Skip check if a good libdir was already found
IF NOT "%libdir%" == "none" GOTO :eof
SET libdir=%CD%\%1
IF NOT EXIST "%libdir%\%driver_lib%.dll"       GOTO :wrongLibDir
IF NOT EXIST "%libdir%\%driver_lib_setup%.dll" GOTO :wrongLibDir
REM ECHO Libdir (%libdir%) is OK.
GOTO :eof
:wrongLibDir
REM ECHO Libdir (%libdir%) is wrong.
SET libdir=none
GOTO :eof

REM ######
REM # A subroutine to compute bindir of the form 
REM # C:\current\working\directory\bin\XXX where XXX is 
REM # the last component of libdir, such as Release, Debug etc.
REM # The libdir should be given as the first argument %1.
REM # Construct %~n1 is used which returns the last component 
REM # ("file name") of the path stored in %1.
REM ######
:subFindBinDir
SET bindir=%CD%\bin\%~n1
GOTO :eof

:doSuccess
ECHO ^+-----------------------------------------------------^+
ECHO ^| DONE                                                ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Hopefully things went well; the Connector/ODBC      ^|
ECHO ^| driver has been registered.                         ^|
ECHO ^|                                                     ^|
ECHO ^| Connector/ODBC is ready to use.                     ^|
ECHO ^|                                                     ^|
ECHO ^| The most common thing to do next is to go to the    ^|
ECHO ^| Control Panel and find the ODBC Administrator -     ^|
ECHO ^| then use it to create a Data Source Name (DSN)      ^|
ECHO ^| so you (and your application) can connect to a      ^|
ECHO ^| MySQL server.                                       ^|
ECHO ^|                                                     ^|
ECHO ^| Alternatively you can use the MyODBC Installer      ^|
ECHO ^| utility to define data sources.                     ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 0

:errorNoDrivers
ECHO ^+-----------------------------------------------------^+
ECHO ^| ERROR                                               ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Could not find Connector/ODBC drivers. Have you run ^|
ECHO ^| this script from the installation directory?        ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 1

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

:errorDriverInstalled
ECHO ^+-----------------------------------------------------^+
ECHO ^| ERROR                                               ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Existing Connector/ODBC installed. Request ignored. ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 1

:errorRegisterDriver
ECHO ^+-----------------------------------------------------^+
ECHO ^| ERROR                                               ^|
ECHO ^+-----------------------------------------------------^+
ECHO ^|                                                     ^|
ECHO ^| Could not register the driver.                      ^|
ECHO ^|                                                     ^|
ECHO ^+-----------------------------------------------------^+
EXIT /B 1
