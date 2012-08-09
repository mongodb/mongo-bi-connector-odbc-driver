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

MACRO(_FIX_NOPREFIX VAR _odbc_config _param)
    EXECUTE_PROCESS(COMMAND ${_odbc_config} ${_param}
                     OUTPUT_VARIABLE _output
                   )
    STRING(REGEX REPLACE "\n" "" _output "${_output}")

    IF(${VAR} MATCHES "/noprefix")

      IF(NOT GUESSED_PREFIX)

        GET_FILENAME_COMPONENT(GUESSED_PREFIX "${_odbc_config}" PATH)
        GET_FILENAME_COMPONENT(GUESSED_PREFIX "${GUESSED_PREFIX}" PATH)
        
      ENDIF(NOT GUESSED_PREFIX)

      STRING(REGEX REPLACE "/noprefix" "${GUESSED_PREFIX}" _output "${_output}")

    ENDIF(${VAR} MATCHES "/noprefix")

    SET(${VAR} ${_output})
ENDMACRO(_FIX_NOPREFIX VAR _odbc_config _param)


IF(ODBC_INCLUDES)

  INCLUDE_DIRECTORIES(${ODBC_INCLUDES})

ENDIF(ODBC_INCLUDES)

IF(ODBC_LIB_DIR)

  SET(ODBC_LINK_FLAGS "-L${ODBC_LIB_DIR} -l${ODBCLIB} -l${ODBCINSTLIB}")

ENDIF(ODBC_LIB_DIR)


# No need to look for (i)odbc[_-]config and run it
IF(NOT ODBC_INCLUDES OR NOT ODBC_LIB_DIR)

  IF(WITH_UNIXODBC)
    # check for location of odbc_config
    FIND_PROGRAM(ODBC_CONFIG odbc_config
          PATHS
          $ENV{ODBC_PATH}/bin)
      
    IF(NOT ODBC_CONFIG)
      MESSAGE(STATUS "Couldn't find unixODBC' odbc_config")

      #odbc config may be not present, and that can be ok
      IF(NOT ODBC_INCLUDES)

        INCLUDE (CheckIncludeFiles)
        CHECK_INCLUDE_FILES(sql.h HAVE_SQL_H)

        IF(NOT HAVE_SQL_H)
          MESSAGE(FATAL_ERROR "sql.h is not found either!")
        ENDIF(NOT HAVE_SQL_H)

      ENDIF(NOT ODBC_INCLUDES)

      
      IF(NOT ODBC_LIB_DIR)

        INCLUDE (CheckLibraryExists)
        CHECK_LIBRARY_EXISTS(odbc SQLConnect "" HAVE_ODBC_LIB)

        IF(NOT HAVE_ODBC_LIB)
          MESSAGE(FATAL_ERROR "odbc lib is not found either!")
        ENDIF(NOT HAVE_ODBC_LIB)

        SET(ODBC_LINK_FLAGS "-l${ODBCLIB} -l${ODBCINSTLIB}")

      ENDIF(NOT ODBC_LIB_DIR)

    ELSE(NOT ODBC_CONFIG)
      MESSAGE(STATUS "unixODBC: Found odbc_config in ${ODBC_CONFIG}")

      IF(NOT ODBC_INCLUDES)
        _FIX_NOPREFIX(ODBC_INCLUDE_DIR ${ODBC_CONFIG} "--include-prefix")
        INCLUDE_DIRECTORIES(${ODBC_INCLUDE_DIR})
      ENDIF(NOT ODBC_INCLUDES)

      IF(NOT ODBC_LIB_DIR)
        _FIX_NOPREFIX(ODBC_LINK_FLAGS ${ODBC_CONFIG} "--libs")
      ENDIF(NOT ODBC_LIB_DIR)

    ENDIF(NOT ODBC_CONFIG)

  ELSE(WITH_UNIXODBC)

    FIND_PROGRAM(ODBC_CONFIG iodbc-config
          PATHS
          $ENV{ODBC_PATH}/bin)
      
    IF(NOT ODBC_CONFIG)
      MESSAGE(FATAL_ERROR "Couldn't find iODBC")
    ENDIF(NOT ODBC_CONFIG)

    MESSAGE(STATUS "iODBC: Found iodbc-config in ${ODBC_CONFIG}")


    IF(NOT ODBC_INCLUDES)
      _FIX_NOPREFIX(ODBC_CFLAGS ${ODBC_CONFIG} "--cflags")
      SET(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")
    ENDIF(NOT ODBC_INCLUDES)

    IF(NOT ODBC_LIB_DIR)
      _FIX_NOPREFIX(ODBC_LINK_FLAGS ${ODBC_CONFIG} "--libs")
    ENDIF(NOT ODBC_LIB_DIR)

  ENDIF(WITH_UNIXODBC)
ENDIF(NOT ODBC_INCLUDES OR NOT ODBC_LIB_DIR)

