@ECHO OFF
REM Copyright (c) 2006, 2007 MySQL AB
REM Use is subject to license terms.
REM
REM The MySQL Connector/ODBC is licensed under the terms of the GPLv2
REM <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most 
REM MySQL Connectors. There are special exceptions to the terms and 
REM conditions of the GPLv2 as it is applied to this software, see the 
REM FLOSS License Exception
REM <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
REM
REM This program is free software; you can redistribute it and/or modify 
REM it under the terms of the GNU General Public License as published 
REM by the Free Software Foundation; version 2 of the License.
REM
REM This program is distributed in the hope that it will be useful, but 
REM WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
REM or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
REM for more details.
REM
REM You should have received a copy of the GNU General Public License along
REM with this program; if not, write to the Free Software Foundation, Inc.,
REM 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

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

