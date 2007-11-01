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

ADD_LIBRARY(myodbc5S SHARED
    dialog/callbacks.cpp
    dialog/callbacks.h
    dialog/utils.cpp
    dialog/utils.h
    odbcdialogparams/main.cpp
    odbcdialogparams/odbcdialogparams.cpp
    odbcdialogparams/odbcdialogparams.h
    odbcdialogparams/odbcdialogparams.rc
    odbcdialogparams/TabCtrl.cpp
    odbcdialogparams/TabCtrl.h
    odbcdialogparams/tooltip.cpp
    odbcdialogparams/connector_odbc_header.bmp
    ConfigDSN.c
    myodbc5S.def
    )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/util)

TARGET_LINK_LIBRARIES(myodbc5S
		myodbc3u ${ODBCLIB} ${ODBCINSTLIB} ${MYSQL_CLIENT_LIBS}
        comctl32 ws2_32)

