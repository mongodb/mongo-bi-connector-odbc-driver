# Copyright (c) 2007, 2011, Oracle and/or its affiliates. All rights reserved.
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

ADD_EXECUTABLE(myodbc-installer myodbc3i.c)

IF(WIN32)
  TARGET_LINK_LIBRARIES(myodbc-installer myodbc3u ${ODBCLIB} ${ODBCINSTLIB}
      ${MYSQL_CLIENT_LIBS})
ELSE(WIN32)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
  TARGET_LINK_LIBRARIES(myodbc-installer myodbc3u ${MYSQL_CLIENT_LIBS} ${CMAKE_THREAD_LIBS_INIT} m)
ENDIF(WIN32)

IF(MYSQL_CXX_LINKAGE)
  SET_TARGET_PROPERTIES(myodbc-installer PROPERTIES LINKER_LANGUAGE CXX)
ENDIF(MYSQL_CXX_LINKAGE)

INSTALL(TARGETS myodbc-installer DESTINATION bin)

