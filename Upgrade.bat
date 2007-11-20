@ECHO OFF
REM #########################################################
REM 
REM \brief  Upgrade an existing install.
REM
REM         Often when testing you want to Uninstall and Install. This
REM         script does this.
REM
REM         Use this upgrade an existing install. This just
REM         calls Uninstall/Install so it has nothing to do
REM         with MS installed software thingy in Control Panel.
REM
REM \sa     README.win
REM
REM #########################################################

CALL Uninstall.bat %1
CALL Install.bat %1

