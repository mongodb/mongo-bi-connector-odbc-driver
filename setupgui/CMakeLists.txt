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

ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

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
ENDIF (WIN32)

ADD_LIBRARY(myodbc5S SHARED ${SETUP_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/util)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/driver)

TARGET_LINK_LIBRARIES(myodbc5S
                      myodbc3u ${ODBCLIB} ${ODBCINSTLIB} ${MYSQL_CLIENT_LIBS}
                      ${PLATFORM_LIBS})
