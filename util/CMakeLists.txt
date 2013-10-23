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

SET(myodbc-util_SRCS stringutil.c
                  stringutil.h
                  unicode_transcode.c
                  installer.c
                  installer.h)

IF(NOT WIN32)
  SET(myodbc-util_SRCS ${myodbc-util_SRCS}
                      odbcinstw.c)
ENDIF(NOT WIN32)

ADD_LIBRARY(myodbc-util STATIC ${myodbc-util_SRCS})

TARGET_LINK_LIBRARIES(myodbc-util ${ODBCINSTLIB})

IF(NOT WIN32)
  SET_TARGET_PROPERTIES(myodbc-util PROPERTIES COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})

  INCLUDE_DIRECTORIES(${DL_INCLUDES})

  TARGET_LINK_LIBRARIES(myodbc-util ${DL_LIBS})

  SET_TARGET_PROPERTIES(myodbc-util PROPERTIES LINK_FLAGS "${DL_LFLAGS}")
ENDIF(NOT WIN32)
