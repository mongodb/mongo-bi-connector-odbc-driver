# Copyright (C) 1995-2007 MySQL AB
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# There are special exceptions to the terms and conditions of the GPL
# as it is applied to this software. View the full text of the exception
# in file LICENSE.exceptions in the top-level directory of this software
# distribution.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

##########################################################################

PROJECT(MYODBC_MSI_INSTALLER)

#-------------- wix installer ------------------------
IF(WIN32)
  INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindWix.cmake)
  # ----------- generate version information ---------
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

  IF(${LICENSE})
    SET(MSI_PACKAGE "mysql-connector-odbc-commercial-${ODBC_VERSION}-${MSI_VERSION}.msi")
  ELSE(${LICENSE})
    SET(MSI_PACKAGE "mysql-connector-odbc-${ODBC_VERSION}-${MSI_VERSION}.msi")
  ENDIF(${LICENSE})

ENDIF(WIN32)
#-----------------------------------------------------
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
	DEPENDS mysql_odbc.xml
                mysql_common_ui.xml
	COMMAND ${WIX_DIR}/candle.exe mysql_odbc.xml -o mysql_odbc.wixobj)
