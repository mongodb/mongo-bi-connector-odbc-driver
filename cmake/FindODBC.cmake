# Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved
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

IF(WITH_UNIXODBC)
	# check for location of odbc_config
	FIND_PROGRAM(ODBC_CONFIG odbc_config
				 $ENV{ODBC_PATH}/bin
				 PATHS)
		
	IF(NOT ODBC_CONFIG)
		MESSAGE(FATAL_ERROR "Couldn't find unixODBC")
	ENDIF(NOT ODBC_CONFIG)

	MESSAGE(STATUS "unixODBC: Found odbc_config in ${ODBC_CONFIG}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--include-prefix" OUTPUT_VARIABLE ODBC_INCLUDE_DIR)
	SET (CMAKE_FLAGS "${CMAKE_FLAGS} -I${ODBC_INCLUDE_DIR}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--libs" OUTPUT_VARIABLE ODBC_LINK_FLAGS)

ELSE(WITH_UNIXODBC)

	FIND_PROGRAM(ODBC_CONFIG iodbc-config
				 $ENV{ODBC_PATH}/bin
				 PATHS)
		
	IF(NOT ODBC_CONFIG)
		MESSAGE(FATAL_ERROR "Couldn't find iODBC")
	ENDIF(NOT ODBC_CONFIG)

	MESSAGE(STATUS "iODBC: Found iodbc-config in ${ODBC_CONFIG}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE ODBC_CFLAGS)
	SET(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")

	EXEC_PROGRAM(${ODBC_CONFIG} ARGS "--libs" OUTPUT_VARIABLE ODBC_LINK_FLAGS)

		
ENDIF(WITH_UNIXODBC)

