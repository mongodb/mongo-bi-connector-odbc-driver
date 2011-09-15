
# Copyright (©) 2007, 2011, Oracle and/or its affiliates. All rights reserved.
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

PROJECT(MySQL_Connector_ODBC)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4 FATAL_ERROR)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#SET(CMAKE_VERBOSE_MAKEFILE ON)

IF(NOT WIN32)
        SET(DISABLE_GUI 1)
ENDIF(NOT WIN32)

#-------------- find  mysql --------------------
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindMySQL.cmake)
#-----------------------------------------------------

#-------------- unixodbc/iodbc/win -------------------
IF(WIN32)
        SET(ODBCLIB odbc32)
        SET(ODBCINSTLIB odbccp32)
ELSE(WIN32)
        IF(WITH_UNIXODBC)
                SET(ODBCLIB odbc)
                SET(ODBCINSTLIB odbcinst)
        ELSE(WITH_UNIXODBC)
                SET(ODBCLIB iodbc)
                SET(ODBCINSTLIB iodbcinst)
        ENDIF(WITH_UNIXODBC)
        # FindODBC uses ODBCLIB and ODBCINSTLIB in some cases
        INCLUDE(cmake/FindODBC.cmake)
        INCLUDE(cmake/FindLtdl.cmake)
ENDIF(WIN32)
#-----------------------------------------------------

#-------- configuring paths to odbc heders for compatibility checks ---------
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/sqlcolattrib1.c.cmake ${CMAKE_BINARY_DIR}/cmake/sqlcolattrib1.c @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/sqlcolattrib2.c.cmake ${CMAKE_BINARY_DIR}/cmake/sqlcolattrib2.c @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/sqlparamopt1.c.cmake ${CMAKE_BINARY_DIR}/cmake/sqlparamopt1.c @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/sqlparamopt2.c.cmake ${CMAKE_BINARY_DIR}/cmake/sqlparamopt2.c @ONLY)
#-----------------------------------------------------

#------------------ check compatibility---------------
TRY_COMPILE(COMPILE_RESULT ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/cmake/sqlcolattrib1.c)
MESSAGE(STATUS "Checking if SQLColAttribute last arg is compatible with SQLLEN* - ${COMPILE_RESULT}")

IF(COMPILE_RESULT)
        ADD_DEFINITIONS(-DUSE_SQLCOLATTRIBUTE_SQLLEN_PTR)
ELSE(COMPILE_RESULT)
        TRY_COMPILE(COMPILE_RESULT1 ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/cmake/sqlcolattrib2.c)
        MESSAGE(STATUS "Checking if SQLColAttribute last arg is compatible with SQLPOINTER - ${COMPILE_RESULT1}")

        IF(COMPILE_RESULT1)
                ADD_DEFINITIONS(-DUSE_SQLCOLATTRIBUTE_SQLPOINTER)
        ELSE(COMPILE_RESULT1)
                # By default using SQLLEN parameter
                ADD_DEFINITIONS(-DUSE_SQLCOLATTRIBUTE_SQLLEN_PTR)
        ENDIF(COMPILE_RESULT1)
ENDIF(COMPILE_RESULT)

TRY_COMPILE(COMPILE_RESULT ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/cmake/sqlparamopt1.c)
MESSAGE(STATUS "Checking if SQLParamOptions() 2nd and 3rd arg is compatible with SQLULEN - ${COMPILE_RESULT}")

IF(COMPILE_RESULT)
        ADD_DEFINITIONS(-DUSE_SQLPARAMOPTIONS_SQLULEN_PTR)
ELSE(COMPILE_RESULT)
        TRY_COMPILE(COMPILE_RESULT1 ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/cmake/sqlparamopt2.c)
        MESSAGE(STATUS "Checking if SQLParamOptions() 2nd and 3rd arg is compatible with SQLUINTEGER - ${COMPILE_RESULT1}")

        IF(COMPILE_RESULT1)
                ADD_DEFINITIONS(-DUSE_SQLPARAMOPTIONS_SQLUINTEGER_PTR)
        ELSE(COMPILE_RESULT1)
                # SQLULEN is a default
		MESSAGE(STATUS "Apparently odbc headers could not be found. 2nd and 3rd parameters assumed to be (*)SQLULEN by default")
                ADD_DEFINITIONS(-DUSE_SQLPARAMOPTIONS_SQLULEN_PTR)
        ENDIF(COMPILE_RESULT1)

ENDIF(COMPILE_RESULT)
#-----------------------------------------------------

	IF(NOT NO_THREADS)
		MESSAGE(STATUS "Enabling threads support")
		ADD_DEFINITIONS(-DTHREAD)
	ENDIF(NOT NO_THREADS)

#------------ build options for windows --------------
IF(WIN32)
        REMOVE_DEFINITIONS(-DUNICODE)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
        ADD_DEFINITIONS(-DENGLISH -DMYODBC_EXPORTS -D_USERDLL)
        ADD_DEFINITIONS(-D_WIN32 -DWIN32 -D_WINDOWS -D__WIN__)

	#Since 5.5.13 libmysql introduces dependency on Secur32.lib
	FILE(STRINGS "${MYSQL_INCLUDE_DIR}\\mysql_version.h" mysql_version REGEX "^\#define[ \t]+MYSQL_VERSION_ID")

	STRING(REGEX REPLACE "^\#define[ \t]+MYSQL_VERSION_ID[ \t]+([0-9]+)" "\\1" MYSQL_CLIENT_VERSION "${mysql_version}")

	IF(MYSQL_CLIENT_VERSION GREATER 50512)
		MESSAGE(STATUS "MySQL client lib(version ${MYSQL_CLIENT_VERSION}) requires Secure32.lib - TRUE")
		SET(SECURE32_LIB Secur32)
	ENDIF(MYSQL_CLIENT_VERSION GREATER 50512)

        # edits for all config build flags
        FOREACH(TYPE C CXX)
                # makefiles use blank configuration
                FOREACH(CFG "_DEBUG" "_MINSIZEREL" "_RELEASE" "_RELWITHDEBINFO")

                        SET(NEW_FLAGS "${CMAKE_${TYPE}_FLAGS${CFG}}")

                        # fix up static libc flags
                        STRING(REPLACE "/MD" "/MT" NEW_FLAGS "${NEW_FLAGS}")

                        # Add some additional help for debug builds
                        IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
                                STRING(REPLACE "/Zi" "/ZI" NEW_FLAGS "${NEW_FLAGS}")
                                SET(NEW_FLAGS "${NEW_FLAGS} /RTC1 /RTCc")
                        ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

                        # *FORCE* to override whats already placed into the cache
                        SET(CMAKE_${TYPE}_FLAGS${CFG} "${NEW_FLAGS}" CACHE STRING
                                "CMAKE_${TYPE}_FLAGS${CFG} (overwritten for odbc)" FORCE)

                ENDFOREACH(CFG)
        ENDFOREACH(TYPE)
ELSE(WIN32)
  ADD_DEFINITIONS(-D_UNIX_)
ENDIF(WIN32)
#-----------------------------------------------------

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(driver)

IF(NOT DISABLE_GUI)
        ADD_SUBDIRECTORY(setupgui)
ENDIF(NOT DISABLE_GUI)

ADD_SUBDIRECTORY(dltest)
ADD_SUBDIRECTORY(installer)

ADD_SUBDIRECTORY(test)

#------------ Installation ---------------------------

IF(WIN32)

# For conveninence copy (un)install.bat scripts to the build
# directory to ease testing.

  CONFIGURE_FILE(Install.bat ${CMAKE_BINARY_DIR}/Install.bat COPYONLY)
  CONFIGURE_FILE(Uninstall.bat ${CMAKE_BINARY_DIR}/Uninstall.bat COPYONLY)

# TODO: line-ending conversions unix->dos

  INSTALL(FILES ChangeLog   DESTINATION . RENAME ChangeLog.txt)
  INSTALL(FILES README      DESTINATION . RENAME README.txt)
  INSTALL(FILES INSTALL     DESTINATION . RENAME INSTALL.txt)
  INSTALL(FILES INSTALL.win DESTINATION . RENAME INSTALL-win.txt)
  INSTALL(FILES COPYING     DESTINATION . RENAME MYODBC5-LICENSE.txt)
  INSTALL(FILES Install.bat   DESTINATION .)
  INSTALL(FILES Uninstall.bat DESTINATION .)

ELSE(WIN32)

  INSTALL(FILES ChangeLog    DESTINATION .)
  INSTALL(FILES README       DESTINATION .)
  INSTALL(FILES README.debug DESTINATION .)
  INSTALL(FILES INSTALL      DESTINATION .)
  INSTALL(FILES COPYING      DESTINATION .)

ENDIF(WIN32)
