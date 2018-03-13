# Copyright (c) 2007, 2012, Oracle and/or its affiliates. All rights reserved.
#
# The MySQL Connector/ODBC is licensed under the terms of the GPLv2
# <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
# MySQL Connectors. There are special exceptions to the terms and
# conditions of the GPLv2 as it is applied to this software, see the
# FLOSS License Exception
# <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

##########################################################################

cmake_minimum_required(VERSION 2.4.6)

PROJECT(MYODBC_MSI_INSTALLER)

#-------------- wix installer ------------------------
IF(WIN32)
  INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindWix.cmake)
  # ----------- generate version information ---------
  MESSAGE(${WIX_DIR})
  IF(WIX_DIR)
    FILE(REMOVE "myodbc_version.xml")
    TRY_RUN(VERSION_OUT VERSION_TEST 
            "${CMAKE_SOURCE_DIR}"
            "${CMAKE_SOURCE_DIR}/cmake/getodbcversion.c" 
            ARGS "myodbc_version.xml")
    IF(NOT VERSION_OUT STREQUAL "0")
        MESSAGE(ERROR "Can't create myodbc_version.xml")
    ENDIF(NOT VERSION_OUT STREQUAL "0")
  ENDIF(WIX_DIR)
  # --------------------------------------------------

  # -------------- find binary resources -------------
  FIND_PATH(WIX_RESOURCE MySQLConnector.ico
          ./resources
          $ENV{WIX_RESOURCE}
          ${CMAKE_SOURCE_DIR}/../../../wix-installer/resources)
  IF(NOT WIX_RESOURCE)
          MESSAGE(ERROR "Can't find binary MySQL resource files. Please specify WIX_RESOURCE")
  ENDIF(NOT WIX_RESOURCE)
  MESSAGE(STATUS "Wix resources found in ${WIX_RESOURCE}")
  # --------------------------------------------------

  # -------------- add wix variables --- -------------
  WRITE_FILE("myodbc_version.xml" "<?define odbc_resource=\"${WIX_RESOURCE}\" ?>" APPEND)
  IF(${MSI_64})
     WRITE_FILE("myodbc_version.xml" "<?define odbc_64bit=1 ?>" APPEND)
     SET(MSI_VERSION "winx64")
  ELSE(${MSI_64})
     WRITE_FILE("myodbc_version.xml" "<?define odbc_64bit=0 ?>" APPEND)
     SET(MSI_VERSION "win32")
  ENDIF(${MSI_64})
  IF(${PDB})
     WRITE_FILE("myodbc_version.xml" "<?define odbc_pdb=1 ?>" APPEND)
  ELSE(${PDB})
     WRITE_FILE("myodbc_version.xml" "<?define odbc_pdb=0 ?>" APPEND)
  ENDIF(${PDB})
  IF(${LICENSE})
     WRITE_FILE("myodbc_version.xml" "<?define license=\"commercial\" ?>" APPEND)
  ELSE(${LICENSE})
     WRITE_FILE("myodbc_version.xml" "<?define license=\"community\" ?>" APPEND)
  ENDIF(${LICENSE})
  WRITE_FILE("myodbc_version.xml" "</Include>" APPEND)
  # --------------------------------------------------
  INCLUDE(${CMAKE_SOURCE_DIR}/myodbc_version.cmake)

  IF(ODBC_VERSION_SUFFIX STREQUAL "a")
	  SET(PACKAGE_SUFFIX "a")
  ENDIF(ODBC_VERSION_SUFFIX STREQUAL "a")

  IF(${LICENSE})
	  SET(MSI_PACKAGE "mysql-connector-odbc-commercial-${ODBC_VERSION}-${MSI_VERSION}${PACKAGE_SUFFIX}.msi")
  ELSE(${LICENSE})
	  SET(MSI_PACKAGE "mysql-connector-odbc-${ODBC_VERSION}-${MSI_VERSION}${PACKAGE_SUFFIX}.msi")
  ENDIF(${LICENSE})

  # Generate GUID
EXECUTE_PROCESS(COMMAND uuidgen
                OUTPUT_VARIABLE CONNECTOR_PKG_ID1)
STRING(STRIP "${CONNECTOR_PKG_ID1}" CONNECTOR_PKG_ID1)
EXECUTE_PROCESS(COMMAND uuidgen
                OUTPUT_VARIABLE CONNECTOR_PKG_ID2)
STRING(STRIP "${CONNECTOR_PKG_ID2}" CONNECTOR_PKG_ID2)

ENDIF(WIN32)
#-----------------------------------------------------

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc/LICENSE.mysql.txt")
  SET(LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/LICENSE.mysql.txt")
  SET(LICENSE_RTF  "${CMAKE_CURRENT_BINARY_DIR}/resources/commercial_license.rtf")
ELSE()
  SET(LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/COPYING.txt")
  SET(LICENSE_RTF  "${CMAKE_CURRENT_BINARY_DIR}/resources/License.rtf")
ENDIF()
FILE(READ ${LICENSE_FILE} CONTENTS)
STRING(REGEX REPLACE "\n" "\\\\par\n" CONTENTS "${CONTENTS}")
STRING(REGEX REPLACE "\t" "\\\\tab" CONTENTS "${CONTENTS}")
FILE(WRITE "${LICENSE_RTF}"
     "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fnil\\fcharset0 Courier New;}}\\viewkind4\\uc1\\pard\\lang1031\\f0\\fs15")
FILE(APPEND "${LICENSE_RTF}" "${CONTENTS}")
FILE(APPEND "${LICENSE_RTF}" "\n}\n")

#-----------------------------------------------------

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mysql_odbc.xml.in
               ${CMAKE_CURRENT_SOURCE_DIR}/mysql-odbc-${ODBC_VERSION}-${MSI_VERSION}.xml @ONLY)

ADD_CUSTOM_TARGET(
        MSI_INSTALLER ALL
        DEPENDS ${MSI_PACKAGE})

ADD_CUSTOM_COMMAND(
        OUTPUT ${MSI_PACKAGE}
        DEPENDS mysql_odbc.wixobj
                mysql_odbc_fragment.wixobj
        COMMAND ${WIX_DIR}/light.exe mysql_odbc.wixobj mysql_odbc_fragment.wixobj -o ${MSI_PACKAGE})

ADD_CUSTOM_COMMAND(
        OUTPUT mysql_odbc_fragment.wixobj
        DEPENDS mysql_odbc_fragment.xml
        COMMAND ${WIX_DIR}/candle.exe mysql_odbc_fragment.xml -o mysql_odbc_fragment.wixobj)

ADD_CUSTOM_COMMAND(
        OUTPUT mysql_odbc.wixobj
        DEPENDS mysql_odbc.xml.in
                mysql_common_ui.xml
        COMMAND ${WIX_DIR}/candle.exe mysql-odbc-${ODBC_VERSION}-${MSI_VERSION}.xml -o mysql_odbc.wixobj)
