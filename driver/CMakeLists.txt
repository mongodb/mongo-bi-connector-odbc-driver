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

# At least in CMake 2.8.12.2 the file "Dawin.cmake" defines
# CMAKE_C_CREATE_SHARED_LIBRARY without honoring
# CMAKE_SHARED_LINKER_FLAGS. I also failed to modify
# CMAKE_C_CREATE_SHARED_LIBRARY to include it. So as a
# work-around we add ODBC_LIB_DIR to the link directories.

IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  LINK_DIRECTORIES(${ODBC_LIB_DIR})
  IF(ODBCINST_LIB_DIR)
    LINK_DIRECTORIES(${ODBCINST_LIB_DIR})
  ENDIF()
ENDIF()

SET(DRIVER_INDEX 0)
SET(DRIVER_LOCATION2 "")
SET(CONNECTOR_DRIVER_TYPE2 "")
SET(CONNECTOR_DRIVER_TYPE_SHORT2 "")

WHILE(${DRIVER_INDEX} LESS ${DRIVERS_COUNT})

  LIST(GET IS_UNICODE_DRIVER ${DRIVER_INDEX} UNICODE)
  LIST(GET CONNECTOR_DRIVER_TYPES_SHORT ${DRIVER_INDEX} CONNECTOR_DRIVER_TYPE_SHORT)
  LIST(GET CONNECTOR_DRIVER_TYPE_LIST ${DRIVER_INDEX} DRIVER_TYPE)

  SET(DRIVER_NAME "myodbc5${CONNECTOR_DRIVER_TYPE_SHORT}")

  SET(DRIVER_SRCS
    catalog.c catalog_no_i_s.c connect.c cursor.c desc.c dll.c error.c execute.c
    handle.c info.c driver.c options.c parse.c prepare.c results.c transact.c
    my_prepared_stmt.c my_stmt.c utility.c)

  IF(UNICODE)
    SET(DRIVER_SRCS ${DRIVER_SRCS} unicode.c)
	  SET(WIDECHARCALL "W")
  ELSE(UNICODE)
    SET(DRIVER_SRCS ${DRIVER_SRCS} ansi.c)
    SET(WIDECHARCALL "")
  ENDIF(UNICODE)

  INCLUDE_DIRECTORIES(../util)

  IF(WIN32)
    # Headers added for convenience of VS users
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/driver/driver.def.cmake ${CMAKE_SOURCE_DIR}/driver/driver${CONNECTOR_DRIVER_TYPE_SHORT}.def @ONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/driver/driver.rc.cmake ${CMAKE_SOURCE_DIR}/driver/driver${CONNECTOR_DRIVER_TYPE_SHORT}.rc @ONLY)
    SET(DRIVER_SRCS ${DRIVER_SRCS} driver${CONNECTOR_DRIVER_TYPE_SHORT}.def driver${CONNECTOR_DRIVER_TYPE_SHORT}.rc catalog.h driver.h
                                   error.h myutil.h parse.h ../MYODBC_MYSQL.h ../MYODBC_CONF.h ../MYODBC_ODBC.h)
  ENDIF(WIN32)

  IF(APPLE)
    ADD_LIBRARY(${DRIVER_NAME} MODULE ${DRIVER_SRCS})
  ELSE(APPLE)
    ADD_LIBRARY(${DRIVER_NAME} SHARED ${DRIVER_SRCS})
  ENDIF(APPLE)

  INSTALL(TARGETS ${DRIVER_NAME} DESTINATION ${LIB_SUBDIR})

IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET_TARGET_PROPERTIES(${DRIVER_NAME} PROPERTIES 
        LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}")
ELSE(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET_TARGET_PROPERTIES(${DRIVER_NAME} PROPERTIES 
        LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}") # -flto
ENDIF()

  IF(UNICODE)
    SET_TARGET_PROPERTIES(${DRIVER_NAME} PROPERTIES
      COMPILE_DEFINITIONS MYODBC_UNICODEDRIVER)
  ENDIF(UNICODE)

  IF(WIN32)
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
      LINK_DIRECTORIES(${MYSQL_DIR}/lib/debug)
      LINK_DIRECTORIES(${MYSQL_DIR}/lib)
    ELSEIF(CMAKE_BUILD_TYPE STREQUAL "")
      LINK_DIRECTORIES(${MYSQL_DIR}/lib/opt)
      LINK_DIRECTORIES(${MYSQL_DIR}/lib)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

    IF (WITH_NODEFAULTLIB)
      SET_TARGET_PROPERTIES(${DRIVER_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "/NODEFAULTLIB:${WITH_NODEFAULTLIB}"
        LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:${WITH_NODEFAULTLIB}"
        LINK_FLAGS_RELEASE "/NODEFAULTLIB:${WITH_NODEFAULTLIB}")
    ENDIF ()

    TARGET_LINK_LIBRARIES(${DRIVER_NAME} myodbc-util
          ${MYSQL_CLIENT_LIBS} ws2_32 ${ODBCINSTLIB} ${SECURE32_LIB})
  ELSE(WIN32)

    IF(DEV_STUDIO_RUNTIME_DIR)
      FOREACH(__runtime_dir IN LISTS DEV_STUDIO_RUNTIME_DIR)
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -R${__runtime_dir}")
      ENDFOREACH()
    ENDIF()

    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_DRIVER_LINK_FLAGS}")
    TARGET_LINK_LIBRARIES(${DRIVER_NAME}
                          ${MYSQL_CLIENT_LIBS} ${CMAKE_THREAD_LIBS_INIT} m)
    TARGET_LINK_LIBRARIES(${DRIVER_NAME} myodbc-util)

    INCLUDE_DIRECTORIES(${DL_INCLUDES})

  ENDIF(WIN32)

  IF (MYSQL_CXX_LINKAGE)
    SET_TARGET_PROPERTIES(${DRIVER_NAME} PROPERTIES 
          LINKER_LANGUAGE CXX
          COMPILE_FLAGS "${MYSQLODBCCONN_COMPILE_FLAGS_ENV} ${MYSQL_CXXFLAGS}")
  ENDIF (MYSQL_CXX_LINKAGE)

  MATH(EXPR DRIVER_INDEX "${DRIVER_INDEX} + 1")

  GET_TARGET_PROPERTY("DRIVER_LOCATION${DRIVER_INDEX}" ${DRIVER_NAME} LOCATION)
  SET("CONNECTOR_DRIVER_TYPE_SHORT${DRIVER_INDEX}" "${CONNECTOR_DRIVER_TYPE_SHORT}")
  SET("CONNECTOR_DRIVER_TYPE${DRIVER_INDEX}" "${DRIVER_TYPE}")

ENDWHILE(${DRIVER_INDEX} LESS ${DRIVERS_COUNT})

# We don't know library location at configuration time(think of debug and release builds)
# Thus we can't just include the script but need post build event

SET(DRIVER_INDEX 0)

WHILE(${DRIVER_INDEX} LESS ${DRIVERS_COUNT})

  LIST(GET CONNECTOR_DRIVER_TYPES_SHORT ${DRIVER_INDEX} CONNECTOR_DRIVER_TYPE_SHORT)
  SET(DRIVER_NAME "myodbc5${CONNECTOR_DRIVER_TYPE_SHORT}")

  ADD_CUSTOM_COMMAND(TARGET ${DRIVER_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
            -DDRIVER_LOCATION1="${DRIVER_LOCATION1}"
            -DDRIVER_LOCATION2="${DRIVER_LOCATION2}"
            -DBINARY_DIR=${CMAKE_BINARY_DIR}/test
            -DCONNECTOR_MAJOR=${CONNECTOR_MAJOR}
            -DCONNECTOR_MINOR=${CONNECTOR_MINOR}
            -DCONNECTOR_DRIVER_TYPE_SHORT1=${CONNECTOR_DRIVER_TYPE_SHORT1}
            -DCONNECTOR_DRIVER_TYPE_SHORT2=${CONNECTOR_DRIVER_TYPE_SHORT2}
            -DCONNECTOR_DRIVER_TYPE1=${CONNECTOR_DRIVER_TYPE1}
            -DCONNECTOR_DRIVER_TYPE2=${CONNECTOR_DRIVER_TYPE2}
            -DDRIVERS_COUNT=${DRIVERS_COUNT}
            -P ${CMAKE_SOURCE_DIR}/test/cmake/generateinifiles.cmake
                  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test")

  MATH(EXPR DRIVER_INDEX "${DRIVER_INDEX} + 1")

ENDWHILE(${DRIVER_INDEX} LESS ${DRIVERS_COUNT})
