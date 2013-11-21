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

SET(DRIVER_SRCS
		ansi.c catalog.c catalog_no_i_s.c connect.c cursor.c desc.c dll.c error.c execute.c
    handle.c info.c driver.c options.c parse.c prepare.c results.c transact.c unicode.c
    my_prepared_stmt.c my_stmt.c utility.c)

INCLUDE_DIRECTORIES(../util)

IF(WIN32)
  # Headers added for convenience of VS users
	SET(DRIVER_SRCS ${DRIVER_SRCS} driver.def driver.rc catalog.h driver.h error.h myutil.h parse.h)
ENDIF(WIN32)

IF(APPLE)
ADD_LIBRARY(myodbc5 MODULE ${DRIVER_SRCS})
ELSE(APPLE)
ADD_LIBRARY(myodbc5 SHARED ${DRIVER_SRCS})
ENDIF(APPLE)

INSTALL(TARGETS myodbc5 DESTINATION ${LIB_SUBDIR})

IF(WIN32)
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    LINK_DIRECTORIES(${MYSQL_DIR}/lib/debug)
    LINK_DIRECTORIES(${MYSQL_DIR}/lib)
  ELSEIF(CMAKE_BUILD_TYPE STREQUAL "")
    LINK_DIRECTORIES(${MYSQL_DIR}/lib/opt)
    LINK_DIRECTORIES(${MYSQL_DIR}/lib)
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

  IF (MYSQLCLIENT_STATIC_LINKING)
    SET_TARGET_PROPERTIES(myodbc5 PROPERTIES
      LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
      LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
      LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
  ENDIF (MYSQLCLIENT_STATIC_LINKING)

	TARGET_LINK_LIBRARIES(myodbc5 myodbc-util
				${MYSQL_CLIENT_LIBS} ${ODBCINSTLIB} ${SECURE32_LIB})
ELSE(WIN32)
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_DRIVER_LINK_FLAGS}")
	TARGET_LINK_LIBRARIES(myodbc5 ${MYSQL_CLIENT_LIBS} ${CMAKE_THREAD_LIBS_INIT} m)
	TARGET_LINK_LIBRARIES(myodbc5 myodbc-util)

  INCLUDE_DIRECTORIES(${DL_INCLUDES})

  #TARGET_LINK_LIBRARIES(myodbc5 ${DL_LIBS})

  #SET_TARGET_PROPERTIES(myodbc5 PROPERTIES LINK_FLAGS "${DL_LFLAGS}")

ENDIF(WIN32)

IF (MYSQL_CXX_LINKAGE)
  SET_TARGET_PROPERTIES(myodbc5 PROPERTIES 
        LINKER_LANGUAGE CXX
        COMPILE_FLAGS "${MYSQLODBCCONN_COMPILE_FLAGS_ENV} ${MYSQL_CXXFLAGS}")
ENDIF (MYSQL_CXX_LINKAGE)


GET_TARGET_PROPERTY(DRIVER_LOCATION myodbc5 LOCATION)

add_custom_command(TARGET myodbc5 POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
		   	    -DDRIVER_LOCATION=${DRIVER_LOCATION}
			    -DBINARY_DIR=${CMAKE_BINARY_DIR}/test
			    -P ${CMAKE_SOURCE_DIR}/test/cmake/generateinifiles.cmake
                   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test")

