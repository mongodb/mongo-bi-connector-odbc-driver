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

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/test)

# put the test exe's in test/
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/test")

IF(EXPOSE_TOFIX_TESTS)
  ADD_DEFINITIONS(-DEXPOSE_TOFIX_TESTS)
ENDIF(EXPOSE_TOFIX_TESTS)


ENABLE_TESTING()

IF(NOT WIN32)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
ENDIF(NOT WIN32)

FOREACH(T my_basics my_blob my_bulk my_catalog1 my_catalog2 my_crash my_curext my_cursor
		my_datetime my_desc my_dyn_cursor my_error my_info my_keys my_param
		my_prepare my_relative my_result1 my_result2 my_scroll my_setup my_tran
		my_types my_unicode my_unixodbc my_use_result my_bug13766 my_pooling)
  IF(WIN32)
    ADD_EXECUTABLE(${T} ${T}.c odbctap.h)
  ELSE(WIN32)
    ADD_EXECUTABLE(${T} ${T}.c)
  ENDIF(WIN32)

  IF(MYSQL_CXX_LINKAGE)
    SET_TARGET_PROPERTIES(${T} PROPERTIES 
          LINKER_LANGUAGE CXX
          LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}"
          COMPILE_FLAGS "${MYSQLODBCCONN_COMPILE_FLAGS_ENV} ${MYSQL_CXXFLAGS}")
  ENDIF(MYSQL_CXX_LINKAGE)

  INSTALL(TARGETS ${T} DESTINATION test COMPONENT tests)
  SET_TARGET_PROPERTIES(${T} PROPERTIES 
      LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}")

  IF(WIN32)
    IF (MYSQLCLIENT_STATIC_LINKING)
      SET_TARGET_PROPERTIES(${T} PROPERTIES
        LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
        LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
        LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
    ENDIF (MYSQLCLIENT_STATIC_LINKING)
  ENDIF(WIN32)

  IF(WIN32)
    TARGET_LINK_LIBRARIES(${T} ${ODBCLIB} ${ODBCINSTLIB})
  ELSE(WIN32)
    TARGET_LINK_LIBRARIES(${T} ${ODBC_LINK_FLAGS} ${ODBCINSTLIB})
  ENDIF(WIN32)
  ADD_TEST(${T} ${T})

ENDFOREACH(T)

# Adding testsuites for testing driver without DM. May be useful to test behaviors that normally are initiated by DM only
# Separate testsuites for ansi and unicode driver are created - we can't use same test for both since there is no DM to convert
# data and to map calls. Excluding OS X so far, since direct linking there is problematic

IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  FOREACH(CONNECTOR_DRIVER_TYPE_SHORT ${CONNECTOR_DRIVER_TYPES_SHORT})

    # Skipping unicode driver so far
    IF (NOT ${CONNECTOR_DRIVER_TYPE_SHORT} STREQUAL "w")
      SET(T "my_nodm${CONNECTOR_DRIVER_TYPE_SHORT}")

      IF(WIN32)
        ADD_EXECUTABLE(${T} ${T}.c odbctap.h)
      ELSE(WIN32)
        ADD_EXECUTABLE(${T} ${T}.c)
      ENDIF(WIN32)

      IF(MYSQL_CXX_LINKAGE)
        SET_TARGET_PROPERTIES(${T} PROPERTIES 
              LINKER_LANGUAGE CXX
              LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}"
              COMPILE_FLAGS "${MYSQLODBCCONN_COMPILE_FLAGS_ENV} ${MYSQL_CXXFLAGS}")
      ENDIF(MYSQL_CXX_LINKAGE)

      INSTALL(TARGETS ${T} DESTINATION test COMPONENT tests)
      SET_TARGET_PROPERTIES(${T} PROPERTIES 
          LINK_FLAGS "${MYSQLODBCCONN_LINK_FLAGS_ENV} ${MYSQL_LINK_FLAGS}")

      IF(WIN32)
        IF (MYSQLCLIENT_STATIC_LINKING)
          SET_TARGET_PROPERTIES(${T} PROPERTIES
            LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
            LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
            LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
        ENDIF (MYSQLCLIENT_STATIC_LINKING)
      ENDIF(WIN32)

      # Not quite clear atm whether ${ODBCINSTLIB} is needed
      TARGET_LINK_LIBRARIES(${T} myodbc5${CONNECTOR_DRIVER_TYPE_SHORT})
      ADD_TEST(${T} ${T})
    ENDIF (NOT ${CONNECTOR_DRIVER_TYPE_SHORT} STREQUAL "w")

  ENDFOREACH(CONNECTOR_DRIVER_TYPE_SHORT)

ENDIF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

TARGET_LINK_LIBRARIES(my_basics ${CMAKE_THREAD_LIBS_INIT})

INSTALL(FILES
	${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake
	${CMAKE_CURRENT_BINARY_DIR}/odbc.ini
	${CMAKE_CURRENT_BINARY_DIR}/odbcinst.ini
        DESTINATION test COMPONENT tests)

