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

SET(myodbc3u_SRCS stringutil.c
                  stringutil.h
                  unicode_transcode.c
                  installer.c
                  installer.h)

IF(USE_LEGACY_ODBC_GUI)
  SET(myodbc3u_SRCS ${myodbc3u_SRCS}
                        MYODBCUtilAllocDataSource.c
                        MYODBCUtilAllocDriver.c
                        MYODBCUtilClearDataSource.c
                        MYODBCUtilClearDriver.c
                        MYODBCUtilDefaultDataSource.c
                        MYODBCUtilDSNExists.c
                        MYODBCUtilFreeDataSource.c
                        MYODBCUtilFreeDriver.c
                        MYODBCUtilGetDataSourceNames.c
                        MYODBCUtilGetDriverNames.c
                        MYODBCUtilGetIniFileName.c
                        MYODBCUtilInsertStr.c
                        MYODBCUtilReadConnectStr.c
                        MYODBCUtilReadDataSource.c
                        MYODBCUtilReadDataSourceStr.c
                        MYODBCUtilReadDriver.c
                        MYODBCUtilWriteConnectStr.c
                        MYODBCUtilWriteDataSource.c
                        MYODBCUtilWriteDataSourceStr.c
                        MYODBCUtilWriteDriver.c)
ENDIF(USE_LEGACY_ODBC_GUI)

IF(NOT WIN32)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

  IF(LTDL_INCLUDES)
    INCLUDE_DIRECTORIES(LTDL_INCLUDES)
  ENDIF(LTDL_INCLUDES)

  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LTDL_LFLAGS}")

ENDIF(NOT WIN32)

ADD_LIBRARY(myodbc3u STATIC ${myodbc3u_SRCS})

TARGET_LINK_LIBRARIES(myodbc3u ${ODBCINSTLIB})

