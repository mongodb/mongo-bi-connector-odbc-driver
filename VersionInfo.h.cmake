/*
  Copyright (c) 2005, 2012, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/* @EDIT_WARNING_MESSAGE@ */
#define SETUP_VERSION         "@CONNECTOR_MAJOR@.@CONNECTOR_MINOR_PADDED@.@CONNECTOR_PATCH_PADDED@"
#define DRIVER_VERSION        "0" SETUP_VERSION

#define MYODBC_VERSION        SETUP_VERSION
#define MYODBC_FILEVER        @CONNECTOR_MAJOR@,@CONNECTOR_MINOR@,@CONNECTOR_PATCH@,0
#define MYODBC_PRODUCTVER     MYODBC_FILEVER
#define MYODBC_STRFILEVER     "@CONNECTOR_MAJOR@, @CONNECTOR_MINOR@, @CONNECTOR_PATCH@, 0\0"
#define MYODBC_STRPRODUCTVER  MYODBC_STRFILEVER

#define MYODBC_STRSERIES      "@CONNECTOR_MAJOR@.@CONNECTOR_MINOR@"
#define MYODBC_STRDRIVERID    MYODBC_STRSERIES"(@CONNECTOR_DRIVER_TYPE_SHORT@)"
#define MYODBC_STRQUALITY     "@CONNECTOR_QUALITY@"
#define MYODBC_STRDRIVERTYPE  "@CONNECTOR_DRIVER_TYPE@"
#define MYODBC_STRTYPE_SUFFIX "@CONNECTOR_DRIVER_TYPE_SHORT@"

#cmakedefine MYODBC_UNICODEDRIVER
