# Copyright (c) 2011, 2013, Oracle and/or its affiliates. All rights reserved.
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


MACRO(_ENV_OR_DEFAULT VAR _default)

  SET(${VAR} $ENV{${VAR}})

  IF(NOT ${VAR})
    SET(${VAR} "${_default}")
  ENDIF(NOT ${VAR})

ENDMACRO(_ENV_OR_DEFAULT VAR _key _default)

MESSAGE(STATUS "Configuring ini files for tests")

SET(TEST_DRIVER1 "${DRIVER_LOCATION1}")
SET(TEST_DRIVER2 "${DRIVER_LOCATION2}")
_ENV_OR_DEFAULT(TEST_DATABASE "test")
_ENV_OR_DEFAULT(TEST_SERVER   "localhost")
_ENV_OR_DEFAULT(TEST_UID      "root")
_ENV_OR_DEFAULT(TEST_PASSWORD "")
_ENV_OR_DEFAULT(TEST_SOCKET   "")

IF(WIN32 AND TEST_SOCKET)
  SET(TEST_OPTIONS "NAMED_PIPE=1")
ENDIF(WIN32 AND TEST_SOCKET)

IF(${DRIVERS_COUNT} LESS 2)
  # If configured to build 1 driver only we make 2nd driver/dsn to work with the same driver
  SET(TEST_DRIVER2 "${DRIVER_LOCATION1}")
  SET(CONNECTOR_DRIVER_TYPE2 "${CONNECTOR_DRIVER_TYPE1}")
  SET(CONNECTOR_DRIVER_TYPE_SHORT2 "")
ENDIF(${DRIVERS_COUNT} LESS 2)

SET(DESCRIPTION1 "MySQL ODBC ${CONNECTOR_MAJOR}.${CONNECTOR_MINOR} ${CONNECTOR_DRIVER_TYPE1} Driver test" )
SET(DESCRIPTION2 "MySQL ODBC ${CONNECTOR_MAJOR}.${CONNECTOR_MINOR} ${CONNECTOR_DRIVER_TYPE2} Driver test" )

CONFIGURE_FILE(odbcinst.ini.in ${BINARY_DIR}/odbcinst.ini @ONLY)
CONFIGURE_FILE(odbc.ini.in     ${BINARY_DIR}/odbc.ini @ONLY)

