@ECHO OFF
REM ########################################################
REM 
REM \brief  Build myodbc.
REM
REM         This exists for those working with the Windows source
REM         distribution.
REM 
REM \sa     README.win
REM
REM #########################################################

ECHO "+-----------------------------------------------------+"
ECHO "| BUILD                                               |"
ECHO "+-----------------------------------------------------+"
REM ****
REM * Removing libraries and executables to force relink...
REM * We do not want to del all in lib as some may be 
REM * additional libs for distro building.
REM ****
IF EXIST lib ( 
    rem rmdir /Q /S lib
    del /Q lib\myodbc*
)
IF EXIST bin ( 
    rmdir /Q /S bin
)

REM Invoke qmake - without relying upon its mechanism to dive into sub dirs.
CALL CreateMakefiles.bat

REM ****
REM * Making the libraries and executables...
REM ****
ECHO Make...
nmake

goto doSuccess

:doSuccess
ECHO "+-----------------------------------------------------+"
ECHO "| BUILD DONE                                          |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| Hopefully things went well and you have results in  |"
ECHO "| lib and bin sub-dirs. Consider running Install.bat  |"
ECHO "| next.                                               |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
EXIT /B 0


