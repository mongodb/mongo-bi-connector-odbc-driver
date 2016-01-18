# Copyright (c) 2007, 2014, Oracle and/or its affiliates. All rights reserved.
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

ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

IF (UNICODE OR NOT ANSI)
  ADD_DEFINITIONS(-DDRIVER_UNICODE)
ELSE (UNICODE OR NOT ANSI)
  ADD_DEFINITIONS(-DDRIVER_ANSI)
ENDIF (UNICODE OR NOT ANSI)


SET(SETUP_SRCS ConfigDSN.c callbacks.c setupgui.h utils.c)

IF (WIN32)
  SET(SETUP_SRCS ${SETUP_SRCS}
                 myodbc5S.def
                 windows/main.cpp
                 windows/odbcdialogparams.cpp
                 windows/odbcdialogparams.h
                 windows/odbcdialogparams.rc
                 windows/resource.h
                 windows/TabCtrl.cpp
                 windows/TabCtrl.h
                 windows/tooltip.cpp
                 windows/connector_odbc_header.bmp)
  SET(PLATFORM_LIBS comctl32)
ELSE (WIN32)
  SET(SETUP_SRCS ${SETUP_SRCS}
                 gtk/odbcdialogparams.c
                 gtk/ODBCINSTGetProperties.c
  )
  INCLUDE (FindPkgConfig)
  PKG_CHECK_MODULES(GTK2 gtk+-2.0)
  IF (GTK2_INCLUDE_DIRS)
    INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
    LINK_DIRECTORIES(${GTK2_LIBRARY_DIRS})
  ELSE (GTK2_INCLUDE_DIRS)
    PKG_CHECK_MODULES(GTK3 gtk+-3.0)
    INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
    LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
  ENDIF (GTK2_INCLUDE_DIRS)
ENDIF (WIN32)


IF (APPLE)
ADD_LIBRARY(myodbc5S MODULE ${SETUP_SRCS})
ELSE (APPLE)
ADD_LIBRARY(myodbc5S SHARED ${SETUP_SRCS})
ENDIF (APPLE)

INSTALL(TARGETS myodbc5S DESTINATION ${LIB_SUBDIR})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/util)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/driver)

IF (NOT WIN32)
  SET_TARGET_PROPERTIES(myodbc5S PROPERTIES COMPILE_FLAGS "${PLATFORM_CFLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
  IF (GTK2_LIBRARIES)
    TARGET_LINK_LIBRARIES(myodbc5S ${GTK2_LIBRARIES})
  ELSE (GTK2_LIBRARIES)
    TARGET_LINK_LIBRARIES(myodbc5S ${GTK3_LIBRARIES})
  ENDIF (GTK2_LIBRARIES)
ENDIF (NOT WIN32)

TARGET_LINK_LIBRARIES(myodbc5S
                      myodbc-util ${ODBCLIB} ${ODBCINSTLIB} ${MYSQL_CLIENT_LIBS}
                      ${PLATFORM_LIBS})

IF (MYSQLCLIENT_STATIC_LINKING AND WIN32)
  SET_TARGET_PROPERTIES(myodbc5S PROPERTIES
    LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
    LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
    LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
ENDIF (MYSQLCLIENT_STATIC_LINKING AND WIN32)

IF (WIN32)
  IF (MYSQLCLIENT_STATIC_LINKING)
    SET_TARGET_PROPERTIES(myodbc5S PROPERTIES
      LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMT"
      LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
      LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
  ENDIF (MYSQLCLIENT_STATIC_LINKING)
ENDIF (WIN32)

IF(MYSQL_CXX_LINKAGE)
  SET_TARGET_PROPERTIES(myodbc5S PROPERTIES LINKER_LANGUAGE CXX)
ENDIF(MYSQL_CXX_LINKAGE)
