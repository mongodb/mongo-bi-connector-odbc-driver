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

SET(DRIVER_SRCS
		ansi.c catalog.c connect.c cursor.c desc.c dll.c error.c execute.c
        handle.c
        info.c driver.c options.c prepare.c results.c transact.c unicode.c
        utility.c)

INCLUDE_DIRECTORIES(../util)

IF(WIN32)
	SET(DRIVER_SRCS ${DRIVER_SRCS} driver.def driver.rc)
ENDIF(WIN32)

ADD_LIBRARY(myodbc5 SHARED ${DRIVER_SRCS})

INSTALL(TARGETS myodbc5 DESTINATION lib)

IF(WIN32)
	TARGET_LINK_LIBRARIES(myodbc5
				user32 advapi32 wsock32 ws2_32 myodbc3u
				mysqlclient ${ODBCLIB} ${ODBCINSTLIB})

	FIND_LIBRARY(MYSQL_ZLIB zlib)
	FIND_LIBRARY(MYSQL_YASSL yassl)
	FIND_LIBRARY(MYSQL_TAOCRYPT taocrypt)
	IF (MYSQL_ZLIB)
		TARGET_LINK_LIBRARIES(myodbc5 zlib)
	ENDIF (MYSQL_ZLIB)
	IF (MYSQL_YASSL)
		TARGET_LINK_LIBRARIES(myodbc5 yassl)
	ENDIF (MYSQL_YASSL)
	IF (MYSQL_TAOCRYPT)
		TARGET_LINK_LIBRARIES(myodbc5 taocrypt)
	ENDIF (MYSQL_TAOCRYPT)
ELSE(WIN32)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
	TARGET_LINK_LIBRARIES(myodbc5 ltdl mysqlclient_r)
	TARGET_LINK_LIBRARIES(myodbc5 myodbc3u)
ENDIF(WIN32)

