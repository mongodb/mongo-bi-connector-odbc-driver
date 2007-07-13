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

ADD_LIBRARY(myodbc3u STATIC MYODBCUtilAllocDataSource.c
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

TARGET_LINK_LIBRARIES(myodbc3u ${ODBCINSTLIB})

