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
REM \brief  Create binary distribution (with installer).
REM
REM         Creates binary distributions with installer. It will
REM         create a GPL and a Commercial version.
REM         1) MSI
REM         2) setup.exe (zipped)
REM 
REM \sa     CreateBinaryZip.bat
REM         CreateSourceZip.bat
REM
REM #########################################################

IF "%1"=="" GOTO doSyntax
IF "%2"=="" GOTO doSyntax

REM Building...
call Build.bat

REM Copying files to wix stage area...
IF NOT EXIST ..\wix-installer\bin\mysql-connector-odbc-%1-win32 (
    mkdir ..\wix-installer\bin\mysql-connector-odbc-%1-win32
)
IF NOT EXIST ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows (
    mkdir ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows
)
IF NOT EXIST ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32 (
    mkdir ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
)
copy bin\* ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy lib\* ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy Licenses_for_Third-Party_Components.txt ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy COPYING   ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32
copy LICENSE.* ..\wix-installer\bin\mysql-connector-odbc-%1-win32\Windows\System32

REM Creating Commercial msi...
cd ..\wix-installer
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\LICENSE.mysql bin\mysql-connector-odbc-%1-win32\Windows\System32\myodbc3-license.rtf
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\LICENSE.mysql resources\commercial_license.rtf
call OdbcMakeSetup.bat %1 %2 commercial

REM Creating GPL msi...
move /Y bin\dist\mysql-connector-odbc-%1-win32.msi bin\dist\mysql-connector-odbc-commercial-%1-win32.msi
move /Y bin\dist\mysql-connector-odbc-%1-win32.msi bin\dist\mysql-connector-odbc-commercial-%1-win32.msi
move /Y bin\dist\mysql-connector-odbc-%1-win32.zip bin\dist\mysql-connector-odbc-commercial-%1-win32.zip
move /Y bin\dist\mysql-connector-odbc-%1-win32.msi.md5 bin\dist\mysql-connector-odbc-commercial-%1-win32.msi.md5
move /Y bin\dist\mysql-connector-odbc-%1-win32.zip.md5 bin\dist\mysql-connector-odbc-commercial-%1-win32.zip.md5
copy bin\mysql-connector-odbc-%1-win32\Windows\System32\COPYING bin\mysql-connector-odbc-%1-win32\Windows\System32\myodbc3-license.rtf
call OdbcMakeSetup.bat %1 %2 gpl

cd ..\*odbc3

EXIT /B 0

:doSyntax
ECHO "+-----------------------------------------------------+"
ECHO "| CreateBinaryMsi.bat                                 |"
ECHO "+-----------------------------------------------------+"
ECHO "|                                                     |"
ECHO "| DESCRIPTION                                         |"
ECHO "|                                                     |"
ECHO "| Use this to build sources and create a MSI based    |"
ECHO "| installers for commercial and GPL.                  |"
ECHO "|                                                     |"
ECHO "| SYNTAX                                              |"
ECHO "|                                                     |"
ECHO "| CreateBinaryMsi <version> <build-type>              |"
ECHO "|                                                     |"
ECHO "| <version>   must be a 3 number version              |"
ECHO "|                                                     |"
ECHO "| <built-type> must be;                               |"
ECHO "|              a - alpha                              |"
ECHO "|              b - beta                               |"
ECHO "|              r - release candidate                  |"
ECHO "|              p - production                         |"
ECHO "|              i - internal                           |"
ECHO "|                                                     |"
ECHO "| EXAMPLE                                             |"
ECHO "|                                                     |"
ECHO "| CreateBinaryMsi 3.51.12 p                           |"
ECHO "|                                                     |"
ECHO "+-----------------------------------------------------+"

