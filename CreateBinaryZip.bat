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
REM \brief  Create binary distribution (without installer).
REM
REM         Creates binary distribution - without installer.
REM         This may be useful for those wishing to incorporate
REM         myodbc into their own installer.
REM 
REM \note   To do this you are going to need pkzipc (the command-line
REM         version of pkzip) somewhere in your path.
REM
REM \sa     CreateBinaryMsi.bat
REM         CreateSourceZip.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax

ECHO Building...
call Build.bat

ECHO Clean any existing stage area...
IF EXIST .\mysql-connector-odbc-noinstall-%1-win32.zip ( 
    del mysql-connector-odbc-noinstall-%1-win32.zip 
)
IF EXIST .\mysql-connector-odbc-noinstall-%1-win32 ( 
    rmdir /S /Q mysql-connector-odbc-noinstall-%1-win32 
)

IF EXIST .\mysql-connector-odbc-commercial-noinstall-%1-win32.zip ( 
    del mysql-connector-odbc-commercial-noinstall-%1-win32.zip 
)
IF EXIST .\mysql-connector-odbc-commercial-noinstall-%1-win32 ( 
    rmdir /S /Q mysql-connector-odbc-commercial-noinstall-%1-win32 
)

ECHO GPL: Create stage area and populate...
mkdir mysql-connector-odbc-noinstall-%1-win32 
mkdir mysql-connector-odbc-noinstall-%1-win32\bin
mkdir mysql-connector-odbc-noinstall-%1-win32\lib
xcopy /E /Y bin\* mysql-connector-odbc-noinstall-%1-win32\bin
xcopy /E /Y lib\* mysql-connector-odbc-noinstall-%1-win32\lib
copy Install.bat mysql-connector-odbc-noinstall-%1-win32
copy Uninstall.bat mysql-connector-odbc-noinstall-%1-win32
copy ChangeLog mysql-connector-odbc-noinstall-%1-win32\ChangeLog.rtf
copy COPYING mysql-connector-odbc-noinstall-%1-win32\COPYING.rtf
copy README mysql-connector-odbc-noinstall-%1-win32\README.rtf
copy INSTALL mysql-connector-odbc-noinstall-%1-win32\INSTALL.rtf
copy INSTALL.win mysql-connector-odbc-noinstall-%1-win32\INSTALL-win.rtf
copy Licenses_for_Third-Party_Components.txt mysql-connector-odbc-noinstall-%1-win32

ECHO Zipping...
pkzipc -add -maximum -recurse -path=current mysql-connector-odbc-noinstall-%1-win32.zip mysql-connector-odbc-noinstall-%1-win32\*.*

ECHO COMMERCIAL: Create stage area and populate...
move mysql-connector-odbc-noinstall-%1-win32 mysql-connector-odbc-commercial-noinstall-%1-win32 
copy LICENSE.mysql mysql-connector-odbc-commercial-noinstall-%1-win32\LICENSE.rtf

ECHO Zipping...
pkzipc -add -maximum -recurse -path=current mysql-connector-odbc-commercial-noinstall-%1-win32.zip mysql-connector-odbc-commercial-noinstall-%1-win32/*.*
IF EXIST .\mysql-connector-odbc-commercial-noinstall-%1-win32 ( 
    rmdir /S /Q mysql-connector-odbc-commercial-noinstall-%1-win32 
)

EXIT /B 0

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| CreateBinaryZip.bat                                 |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to build sources and create a ZIP based    |"
ECHO "| noinstaller distribution for commercial and GPL.    |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| CreateBinaryZip <version>                           |"
ECHO "|                                                     |"
ECHO "| <version>   must be a 3 number version              |"
ECHO "|                                                     |"
ECHO "| EXAMPLE                                             |"
ECHO "|                                                     |"
ECHO "| CreateBinaryZip 3.51.12                             |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"

