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

PROJECT(MySQL_Connector_ODBC)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4 FATAL_ERROR)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

#-------------- find qt and mysql --------------------
IF(NOT DISABLE_GUI)
	INCLUDE(FindQt)

	INCLUDE_DIRECTORIES(${QT_INCLUDES})
	LINK_DIRECTORIES(${QT_LIBRARY_DIR})

	# Need explicit Qt deps on Win w/CMake < 2.4.6
	# TODO double check if it's 2.4.6 or 2.4.5 that has the DEPENDENCIES vars
	# (and fix below)
	IF(WIN32 AND
			CMAKE_MAJOR_VERSION EQUAL 2 AND
			CMAKE_MINOR_VERSION EQUAL 4 AND
			CMAKE_PATCH_VERSION LESS  6)
		# not exactly the same as FindQt4.cmake, but good enough
		SET(QT_CORE_LIB_DEPENDENCIES Imm32.lib Winmm.lib Ws2_32.lib)
	ENDIF(WIN32 AND
			CMAKE_MAJOR_VERSION EQUAL 2 AND
			CMAKE_MINOR_VERSION EQUAL 4 AND
			CMAKE_PATCH_VERSION LESS  6)

	# FindQT4.cmake is supposed to set this, but doesn't seem to, so we do it
	IF (CMAKE_BUILD_TYPE STREQUAL Debug)
		IF (NOT QT_LIBRARIES)
			SET(QT_LIBRARIES ${QT_QTCORE_LIBRARY_DEBUG}
					${QT_QTGUI_LIBRARY_DEBUG} ${QT_QT3SUPPORT_LIBRARY_DEBUG}
					${QT_CORE_LIB_DEPENDENCIES} ${QT_GUI_LIB_DEPENDENCIES})
		ENDIF (NOT QT_LIBRARIES)
	ELSE (CMAKE_BUILD_TYPE STREQUAL Debug)
		IF (NOT QT_LIBRARIES)
			SET(QT_LIBRARIES ${QT_QTCORE_LIBRARY_RELEASE}
					${QT_QTGUI_LIBRARY_RELEASE} ${QT_QT3SUPPORT_LIBRARY_RELEASE}
					${QT_CORE_LIB_DEPENDENCIES} ${QT_GUI_LIB_DEPENDENCIES})
		ENDIF (NOT QT_LIBRARIES)
	ENDIF (CMAKE_BUILD_TYPE STREQUAL Debug)
ENDIF(NOT DISABLE_GUI)

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/FindMySQL.cmake)
#-----------------------------------------------------

#-------------- unixodbc/iodbc/win -------------------
IF(WIN32)
	SET(ODBCLIB odbc32)
	SET(ODBCINSTLIB odbccp32)
ELSE(WIN32)
	INCLUDE(cmake/FindODBC.cmake)
	SET(ODBCLIB odbc)
	SET(ODBCINSTLIB odbcinst)
ENDIF(WIN32)
#-----------------------------------------------------

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

#------------ build options for windows --------------
IF(WIN32)
	REMOVE_DEFINITIONS(-DUNICODE)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
	ADD_DEFINITIONS(-DENGLISH -DMYODBC_EXPORTS -D_USERDLL)
	ADD_DEFINITIONS(-D_WIN32 -DWIN32 -D_WINDOWS -D__WIN__)

	# edits for all config build flags
	FOREACH(TYPE C CXX)
		# makefiles use blank configuration
		FOREACH(CFG "_DEBUG" "_MINSIZEREL" "_RELEASE" "_RELWITHDEBINFO")
		#FOREACH(CFG "" ${CMAKE_CONFIGURATION_TYPES})
			#MESSAGE("Replacing CMAKE_${TYPE}_FLAGS${CFG}: ${CMAKE_${TYPE}_FLAGS${CFG}}")

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

			#MESSAGE("New       CMAKE_${TYPE}_FLAGS${CFG}: ${CMAKE_${TYPE}_FLAGS${CFG}}")
		ENDFOREACH(CFG)
	ENDFOREACH(TYPE)
ENDIF(WIN32)
#-----------------------------------------------------

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")

ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(driver)

IF(NOT DISABLE_GUI)
	ADD_SUBDIRECTORY(setup)
ENDIF(NOT DISABLE_GUI)

ADD_SUBDIRECTORY(dltest)
ADD_SUBDIRECTORY(installer)
ADD_SUBDIRECTORY(monitor)

IF(NOT DISABLE_GUI)
	ADD_SUBDIRECTORY(dsn-editor)
ENDIF(NOT DISABLE_GUI)

ADD_SUBDIRECTORY(test)

