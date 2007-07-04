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

