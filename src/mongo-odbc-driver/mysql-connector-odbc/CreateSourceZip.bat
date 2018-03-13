@ECHO OFF

REM  Copyright (c) 2006, 2011, Oracle and/or its affiliates. All rights reserved.
REM
REM  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
REM  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
REM  MySQL Connectors. There are special exceptions to the terms and
REM  conditions of the GPLv2 as it is applied to this software, see the
REM  FLOSS License Exception
REM  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
REM
REM  This program is free software; you can redistribute it and/or modify
REM  it under the terms of the GNU General Public License as published
REM  by the Free Software Foundation; version 2 of the License.
REM
REM  This program is distributed in the hope that it will be useful, but
REM  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
REM  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
REM  for more details.
REM
REM  You should have received a copy of the GNU General Public License along
REM  with this program; if not, write to the Free Software Foundation, Inc.,
REM  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
REM

REM #########################################################
REM 
REM \brief  Create source distribution.
REM
REM         Creates source distribution for windows.
REM 
REM \note   To do this you are going to need pkzipc (the command-line
REM         version of pkzip) somewhere in your path.
REM
REM \sa     CreateBinaryMsi.bat
REM         CreateBinaryZip.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax

ECHO Clean any existing stage area...
IF EXIST mysql-connector-odbc-%1-win-src.zip ( 
    del mysql-connector-odbc-%1-win-src.zip 
)
IF EXIST mysql-connector-odbc-%1-win-src ( 
    rmdir /S /Q mysql-connector-odbc-%1-win-src 
)

ECHO Create stage area and populate...
mkdir mysql-connector-odbc-%1-win-src 

REM Its easier to copy specific files then try 
REM to clean out garbage (in this case).
REM xcopy /E /Y * mysql-connector-odbc-%1-win-src

REM root
mkdir mysql-connector-odbc-%1-win-src 
copy ChangeLog mysql-connector-odbc-%1-win-src\ChangeLog.rtf
copy COPYING mysql-connector-odbc-%1-win-src\COPYING.rtf
copy Build.bat mysql-connector-odbc-%1-win-src
copy Install.bat mysql-connector-odbc-%1-win-src
copy Uninstall.bat mysql-connector-odbc-%1-win-src
copy CreateMakefiles.bat mysql-connector-odbc-%1-win-src
copy CreateVisualStudioProjects.bat mysql-connector-odbc-%1-win-src
copy config.pri mysql-connector-odbc-%1-win-src
copy defines.pri mysql-connector-odbc-%1-win-src
copy mysql.pri mysql-connector-odbc-%1-win-src
copy odbc.pri mysql-connector-odbc-%1-win-src
copy root.pro mysql-connector-odbc-%1-win-src
copy README mysql-connector-odbc-%1-win-src\README.rtf
copy BUILD mysql-connector-odbc-%1-win-src\BUILD.rtf
copy BUILD.win mysql-connector-odbc-%1-win-src\BUILD-win.rtf
copy INSTALL mysql-connector-odbc-%1-win-src\INSTALL.rtf
copy INSTALL.win mysql-connector-odbc-%1-win-src\INSTALL-win.rtf
copy Licenses_for_Third-Party_Components.txt mysql-connector-odbc-%1-win-src
copy resource.h mysql-connector-odbc-%1-win-src
copy VersionInfo.h mysql-connector-odbc-%1-win-src
copy mysql.bmp mysql-connector-odbc-%1-win-src

REM dltest
mkdir mysql-connector-odbc-%1-win-src\dltest
copy dltest\*.pro mysql-connector-odbc-%1-win-src\dltest
copy dltest\*.c mysql-connector-odbc-%1-win-src\dltest

REM util
mkdir mysql-connector-odbc-%1-win-src\util
copy util\*.pro mysql-connector-odbc-%1-win-src\util
copy util\*.h mysql-connector-odbc-%1-win-src\util
copy util\*.c mysql-connector-odbc-%1-win-src\util

REM setup
mkdir mysql-connector-odbc-%1-win-src\setup
copy setup\*.pro mysql-connector-odbc-%1-win-src\setup
copy setup\*.def mysql-connector-odbc-%1-win-src\setup
copy setup\*.xpm mysql-connector-odbc-%1-win-src\setup
copy setup\*.rc mysql-connector-odbc-%1-win-src\setup
copy setup\*.c mysql-connector-odbc-%1-win-src\setup
copy setup\*.cpp mysql-connector-odbc-%1-win-src\setup
copy setup\*.h mysql-connector-odbc-%1-win-src\setup

REM installer
mkdir mysql-connector-odbc-%1-win-src\installer
copy installer\*.pro mysql-connector-odbc-%1-win-src\installer
copy installer\*.c mysql-connector-odbc-%1-win-src\installer

REM driver
mkdir mysql-connector-odbc-%1-win-src\driver
copy driver\*.pro mysql-connector-odbc-%1-win-src\driver
copy driver\*.c mysql-connector-odbc-%1-win-src\driver
copy driver\*.h mysql-connector-odbc-%1-win-src\driver
copy driver\*.rc mysql-connector-odbc-%1-win-src\driver
copy driver\*.def mysql-connector-odbc-%1-win-src\driver

ECHO Zipping...
pkzipc -add -maximum -recurse -path=current mysql-connector-odbc-%1-win-src.zip mysql-connector-odbc-%1-win-src/*.*
IF EXIST mysql-connector-odbc-%1-win-src ( 
    rmdir /S /Q mysql-connector-odbc-%1-win-src
)

ECHO ****
ECHO * Hopefully things went well and you have a fresh new source distribution
ECHO * in the source root directory. 
ECHO ****

EXIT /B 0

:doSyntax
ECHO CreateSourceZip
ECHO ===============
ECHO .
ECHO Usage:
ECHO .
ECHO %0 VERSION
ECHO .
ECHO   VERSION      must be; a 3 number version code
ECHO .
ECHO   Examples:
ECHO .
ECHO   %0 3.51.12

