# Copyright (c) 2007, 2013, Oracle and/or its affiliates. All rights reserved.
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

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/util)

IF(DL_INCLUDES)
  INCLUDE_DIRECTORIES(${DL_INCLUDES})
ENDIF(DL_INCLUDES)

ADD_EXECUTABLE(myodbc-installer myodbc-installer.c)

IF(WIN32)
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    LINK_DIRECTORIES(${MYSQL_DIR}/lib/debug)
    LINK_DIRECTORIES(${MYSQL_DIR}/lib)
  ELSEIF(CMAKE_BUILD_TYPE STREQUAL "")
    LINK_DIRECTORIES(${MYSQL_DIR}/lib/opt)
    LINK_DIRECTORIES(${MYSQL_DIR}/lib)
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

  TARGET_LINK_LIBRARIES(myodbc-installer myodbc-util ${ODBCLIB} ${ODBCINSTLIB}
      ${MYSQL_CLIENT_LIBS})
  IF (MYSQLCLIENT_STATIC_LINKING)
    SET_TARGET_PROPERTIES(myodbc-installer PROPERTIES
      LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
      LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
      LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
  ENDIF (MYSQLCLIENT_STATIC_LINKING)
  SET_TARGET_PROPERTIES(myodbc-installer PROPERTIES 
        LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}")
ELSE(WIN32)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
  TARGET_LINK_LIBRARIES(myodbc-installer myodbc-util  
      ${ODBC_LINK_FLAGS} ${MYSQL_CLIENT_LIBS} ${CMAKE_THREAD_LIBS_INIT} m)
  SET_TARGET_PROPERTIES(myodbc-installer PROPERTIES 
        LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}")
ENDIF(WIN32)

IF(MYSQL_CXX_LINKAGE)
  SET_TARGET_PROPERTIES(myodbc-installer PROPERTIES 
        LINKER_LANGUAGE CXX
        COMPILE_FLAGS "${MYSQLODBCCONN_COMPILE_FLAGS_ENV} ${MYSQL_CXXFLAGS}")
ENDIF(MYSQL_CXX_LINKAGE)

INSTALL(TARGETS myodbc-installer DESTINATION bin)

