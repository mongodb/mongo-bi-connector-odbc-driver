@ECHO OFF
REM #########################################################
REM 
REM \brief  Upgrade an existing install.
REM
REM         Often when testing you want to Uninstall and Install. This
REM         script does this.
REM
REM \sa     README.win
REM
REM #########################################################


IF "%1"=="1" GOTO :doIt
IF "%1"=="0" GOTO :doIt

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| Upgrade.bat                                         |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this upgrade an existing install. This just     |"
ECHO "| calls Uninstall/Install so it has nothing to do     |"
ECHO "| with MS installed software thingy in Control Panel. |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| Upgrade <debug>                                     |"
ECHO "|                                                     |"
ECHO "| <debug>  must be;                                   |"
ECHO "|              0 - to install a regular build         |"
ECHO "|              1 - to install a debug version         |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"
EXIT /B 1

:doIt
CALL Uninstall.bat %1
CALL Install.bat %1


