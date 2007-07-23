ECHO OFF
REM #########################################################
REM 
REM \brief  Create Makefiles from qmake project files.
REM
REM         Actually; you only want to use this to 'regenerate'
REM         the Makefiles. So do a qmake from the root source for
REM         the first time and then after that you probably want
REM         to use this for regenerating them due to changes in 
REM         a qmake project file.
REM
REM         It would appear that under certian circumtsances existing
REM         make files will not get regenerated after a qmake project
REM         file has changed.
REM 
REM \note   Its probably a good idea to run this twice as I think the
REM         the problem only occurs the first time an attempt is made
REM         to auto recreate the makefile. 
REM 
REM #########################################################

ECHO ****
ECHO * Recreating Makefiles...
ECHO ****
qmake

ECHO dltest...
cd dltest
qmake
cd ..

ECHO util...
cd util
qmake
cd ..

ECHO setup
cd setup
qmake
cd ..

ECHO installer...
cd installer
qmake
cd ..

ECHO monitor...
cd monitor
qmake
cd ..

ECHO dsn-editor...
cd dsn-editor
qmake
cd ..

ECHO driver...
cd driver
qmake
cd ..

ECHO ****
ECHO * You should now have fresh makefiles to work with. Any changes to
ECHO * qmake project files should be reflected in these new makefiles.
ECHO ****


