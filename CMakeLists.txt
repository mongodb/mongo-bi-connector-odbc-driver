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

#-------------- detect platform ---------------------
IF (${CMAKE_SHARED_LIBRARY_SUFFIX} MATCHES ".dylib")
	SET(ODBC_MAC 1)
	MESSAGE(STATUS "platform settings: Mac")
ENDIF (${CMAKE_SHARED_LIBRARY_SUFFIX} MATCHES ".dylib")
IF (${CMAKE_SHARED_LIBRARY_SUFFIX} MATCHES ".so")
	SET(ODBC_UNIX 1)
	ADD_DEFINITIONS(-D_UNIX_)
	MESSAGE(STATUS "platform settings: *nix")
ENDIF (${CMAKE_SHARED_LIBRARY_SUFFIX} MATCHES ".so")
IF (${CMAKE_SHARED_LIBRARY_SUFFIX} MATCHES ".dll")
	SET(ODBC_WINDOWS 1)
	MESSAGE(STATUS "platform settings: Windows")
ENDIF (${CMAKE_SHARED_LIBRARY_SUFFIX} MATCHES ".dll")
#-----------------------------------------------------

#-------------- find qt and mysql --------------------
IF(NOT DISABLE_GUI)
	IF (ODBC_WINDOWS)
		INCLUDE(FindQt4)
	ELSE(ODBC_WINDOWS)
		INCLUDE(FindQt)
	ENDIF (ODBC_WINDOWS)
	INCLUDE_DIRECTORIES(${QT_INCLUDES})
	LINK_DIRECTORIES(${QT_LIBRARY_DIR})
ENDIF(NOT DISABLE_GUI)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindMySQL.cmake)
#-----------------------------------------------------

#-------------- unixodbc/iodbc? ---------------------
IF(NOT ODBC_WINDOWS)
	IF(WITH_UNIXODBC)
		ADD_DEFINITIONS(-DUSE_UNIXODBC)
	ELSE(WITH_UNIXODBC)
		ADD_DEFINITIONS(-DUSE_IODBC)
	ENDIF(WITH_UNIXODBC)
	INCLUDE(cmake/FindODBC.cmake)
ENDIF(NOT ODBC_WINDOWS)
#-----------------------------------------------------
IF(ODBC_WINDOWS)
	REMOVE_DEFINITIONS(-DUNICODE)
ENDIF(ODBC_WINDOWS)


#------------------ check compatibility---------------
TRY_COMPILE(COMPILE_RESULT ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cmake/sqlcolattrib1.c)
MESSAGE(STATUS "Checking if SQLColAttribute last arg is compatible with SQLLEN* - ${COMPILE_RESULT}")

IF(COMPILE_RESULT)
	ADD_DEFINITIONS(-DUSE_SQLCOLATTRIBUTE_SQLLEN_PTR)
ELSE(COMPILE_RESULT)
	TRY_COMPILE(COMPILE_RESULT1 ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cmake/sqlcolattrib2.c)
	MESSAGE(STATUS "Checking if SQLColAttribute last arg is compatible with SQLPOINTER - ${COMPILE_RESULT1}")

	IF(COMPILE_RESULT1)
		ADD_DEFINITIONS(-DUSE_SQLCOLATTRIBUTE_SQLPOINTER)
	ENDIF(COMPILE_RESULT1)
ENDIF(COMPILE_RESULT)

TRY_COMPILE(COMPILE_RESULT ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cmake/sqlparamopt1.c)
MESSAGE(STATUS "Checking if SQLParamOptions() 2nd and 3rd arg is compatible with SQLULEN - ${COMPILE_RESULT}")

IF(COMPILE_RESULT)
	ADD_DEFINITIONS(-DUSE_SQLPARAMOPTIONS_SQLULEN_PTR)
ELSE(COMPILE_RESULT)
	TRY_COMPILE(COMPILE_RESULT1 ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cmake/sqlparamopt2.c)
	MESSAGE(STATUS "Checking if SQLParamOptions() 2nd and 3rd arg is compatible with SQLUINTEGER - ${COMPILE_RESULT1}")

	IF(COMPILE_RESULT1)
	ADD_DEFINITIONS(-DUSE_SQLPARAMOPTIONS_SQLUINTEGER_PTR)
	ENDIF(COMPILE_RESULT1)

ENDIF(COMPILE_RESULT)
#-----------------------------------------------------

IF(CMAKE_GENERATOR MATCHES "Visual Studio 8 2005 Win64")
  SET(PROCESSOR_ARCH "X64") 
ELSE(CMAKE_GENERATOR MATCHES "Visual Studio 8 2005 Win64")
  SET(PROCESSOR_ARCH "X86")
ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 8 2005 Win64")

IF(ODBC_WIN)
	ADD_DEFINITIONS("-D_WINDOWS -D__WIN__ -D_WIN32 -D_CRT_SECURE_NO_DEPRECATE")
ENDIF(ODBC_WIN)

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")

ENABLE_TESTING()

ADD_SUBDIRECTORY(MYODBCDbg)
ADD_SUBDIRECTORY(dltest)

ADD_SUBDIRECTORY(driver)
ADD_SUBDIRECTORY(util)

IF(NOT DISABLE_GUI)
	ADD_SUBDIRECTORY(setup)
ENDIF(NOT DISABLE_GUI)

ADD_SUBDIRECTORY(installer)
ADD_SUBDIRECTORY(monitor)

IF(NOT DISABLE_GUI)
	ADD_SUBDIRECTORY(dsn-editor)
ENDIF(NOT DISABLE_GUI)

ADD_SUBDIRECTORY(test)
