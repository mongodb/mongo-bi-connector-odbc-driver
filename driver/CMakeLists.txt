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

INCLUDE_DIRECTORIES(/usr/local/mysql/include/mysql)

IF(ODBC_WINDOWS)
	ADD_DEFINITIONS(-D__WIN__ -DENGLISH -DMYODBC_EXPORTS -D_USERDLL -DDONT_DEFINE_VOID)
	ADD_DEFINITIONS(-D_WIN32 -DWIN32 -D_WINDOWS -D__WIN__)
	STRING(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
	STRING(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:msvcrt")
ENDIF(ODBC_WINDOWS)


SET(DRIVER_SRCS
	catalog.c connect.c cursor.c dll.c error.c execute.c handle.c info.c myodbc3.c 
	options.c prepare.c results.c transact.c utility.c 
    ../util/MYODBCUtilAllocDataSource.c ../util/MYODBCUtilAllocDriver.c         
	../util/MYODBCUtilClearDataSource.c ../util/MYODBCUtilClearDriver.c          
	../util/MYODBCUtilDefaultDataSource.c ../util/MYODBCUtilDSNExists.c
	../util/MYODBCUtilFreeDataSource.c ../util/MYODBCUtilFreeDriver.c
	../util/MYODBCUtilGetDataSourceNames.c ../util/MYODBCUtilGetDriverNames.c
	../util/MYODBCUtilGetIniFileName.c ../util/MYODBCUtilInsertStr.c     
	../util/MYODBCUtilReadConnectStr.c ../util/MYODBCUtilReadDataSource.c       
	../util/MYODBCUtilReadDataSourceStr.c ../util/MYODBCUtilReadDriver.c 
	../util/MYODBCUtilWriteConnectStr.c ../util/MYODBCUtilWriteDataSource.c     
	../util/MYODBCUtilWriteDataSourceStr.c ../util/MYODBCUtilWriteDriver.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgConnectAttrString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgConnectOptionString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgDiagFieldString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgEnvAttrString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgFunctionsString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgHandleTypeString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgInfoTypeString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgPosTypeString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgReturnString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgStmtAttrString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgStmtOptionString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgStmtTypeString.c
	../MYODBCDbg/MYODBCDbgLib/MYODBCDbgTransactionTypeString.c)

IF(ODBC_WINDOWS)
	SET(DRIVER_SRCS ${DRIVER_SRCS} myodbc3.def myodbc3.rc)
ENDIF(ODBC_WINDOWS)

ADD_LIBRARY(myodbc3 SHARED ${DRIVER_SRCS})

IF(ODBC_UNIX)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
	TARGET_LINK_LIBRARIES(myodbc3 ltdl mysqlclient_r)
	TARGET_LINK_LIBRARIES(myodbc3 myodbc3u)
ENDIF(ODBC_UNIX)

IF (ODBC_WINDOWS)
	TARGET_LINK_LIBRARIES(myodbc3 user32 advapi32 wsock32 odbc32 odbccp32 ws2_32 myodbc3u
						  mysqlclient)

	FIND_LIBRARY(MYSQL_ZLIB zlib)
	FIND_LIBRARY(MYSQL_YASSL yassl)
	FIND_LIBRARY(MYSQL_TAOCRYPT taocrypt)
	IF (MYSQL_ZLIB)
		TARGET_LINK_LIBRARIES(myodbc3 zlib)
	ENDIF (MYSQL_ZLIB)
	IF (MYSQL_YASSL)
		TARGET_LINK_LIBRARIES(myodbc3 yassl)
	ENDIF (MYSQL_YASSL)
	IF (MYSQL_TAOCRYPT)
		TARGET_LINK_LIBRARIES(myodbc3 taocrypt)
	ENDIF (MYSQL_TAOCRYPT)

ENDIF (ODBC_WINDOWS)





