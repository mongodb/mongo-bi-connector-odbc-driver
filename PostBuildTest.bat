@ECHO OFF
REM #########################################################
REM 
REM \brief  Execute post build tests.
REM
REM         This executes a series of post-build tests. This is
REM         useful for a small sanity check before proceeding with
REM         more extensive tests.
REM
REM         This is not automatically executed after each build.
REM         Typical usage would be to;
REM
REM         > Uninstall.bat
REM         > Build.bat
REM         > Install.bat
REM         > PostBuildTest.bat
REM
REM \sa     Build.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax

ECHO BEGIN: Post build test...
GOTO d
test\bin\connect.exe %1 %2 %3
test\bin\basics.exe %1 %2 %3
test\bin\param.exe %1 %2 %3
test\bin\result.exe %1 %2 %3
test\bin\cursor.exe %1 %2 %3
test\bin\tran.exe %1 %2 %3
test\bin\position.exe %1 %2 %3
test\bin\relative.exe %1 %2 %3
test\bin\scroll.exe %1 %2 %3
test\bin\col_length.exe %1 %2 %3
test\bin\blob.exe %1 %2 %3
test\bin\bulk.exe %1 %2 %3
test\bin\unixodbc.exe %1 %2 %3
test\bin\dyn_cursor.exe %1 %2 %3
test\bin\timestamp.exe %1 %2 %3
test\bin\keys.exe %1 %2 %3
test\bin\curext.exe %1 %2 %3
test\bin\error.exe %1 %2 %3
test\bin\tran_ext.exe %1 %2 %3
REM test\bin\use_result.exe 10000 %1 %2 %3
test\bin\use_result.exe 100 %1 %2 %3
test\bin\catalog.exe %1 %2 %3
:d
test\bin\test32.exe %1 %2 %3
EXIT /B 0

ECHO END: Post build test.

EXIT /B 0

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| PostBuildTest.bat                                   |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to execute a series of post-build tests.   |"
ECHO "| This is often done after a Build.bat.               |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| PostBuildTest <DSN> [UID] [PWD]                     |"
ECHO "|                                                     |"
ECHO "| EXAMPLE                                             |"
ECHO "|                                                     |"
ECHO "| PostBuildTest test                                  |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"



