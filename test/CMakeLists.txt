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
		my_types my_unicode my_unixodbc my_use_result my_bug13766 my_auth)
	ADD_EXECUTABLE(${T} ${T}.c)

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
        LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMT"
        LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:LIBCMT"
        LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMT")
    ENDIF (MYSQLCLIENT_STATIC_LINKING)
	  TARGET_LINK_LIBRARIES(${T} ${ODBCLIB} ${ODBCINSTLIB})
  ELSE(WIN32)
    TARGET_LINK_LIBRARIES(${T} ${ODBC_LINK_FLAGS} ${ODBCINSTLIB})
	ENDIF(WIN32)
	
	ADD_TEST(${T} ${T})
ENDFOREACH(T)

TARGET_LINK_LIBRARIES(my_basics ${CMAKE_THREAD_LIBS_INIT})

INSTALL(FILES
	${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake
	${CMAKE_CURRENT_BINARY_DIR}/odbc.ini
	${CMAKE_CURRENT_BINARY_DIR}/odbcinst.ini
        DESTINATION test COMPONENT tests)


