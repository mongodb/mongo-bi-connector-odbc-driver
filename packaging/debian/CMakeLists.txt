#
# Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.
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

# This is a small CMake project to copy this directory to the top of
# the source, and at the same time do some modifications to these
# files

cmake_minimum_required(VERSION 2.8.5)
project(MySQL_Connector_ODBC_Deb_Init NONE)

# ----------------------------------------------------------------------
# Set some variables to replace
# Use the version variables from "version.cmake"
# ----------------------------------------------------------------------

include(../../version.cmake)

set(YEAR      2016)  # FIXME automate somehow
set(PRODUCT   "MySQL Connector/ODBC ${CONNECTOR_BASE_VERSION}")

if(NOT DEBIAN_REVISION)
  set(DEBIAN_REVISION 1)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/../../LICENSE.mysql")
  set(LIC_FILE "LICENSE.mysql")
  set(VERSION "${CONNECTOR_VERSION}+commercial-${DEBIAN_REVISION}")
else()
  set(LIC_FILE "COPYING")
  set(VERSION "${CONNECTOR_VERSION}-${DEBIAN_REVISION}")
endif()

# ----------------------------------------------------------------------
# Find out if Debian/Ubuntu, the codename, distribution and version
# ----------------------------------------------------------------------

execute_process(
  COMMAND lsb_release --short --id
  OUTPUT_VARIABLE lsb_id
  RESULT_VARIABLE lsb_result
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT lsb_result EQUAL 0)
  message(FATAL_ERROR "Can't run lsb_release")
endif()

string(TOLOWER "${lsb_id}" lsb_id)

if(NOT lsb_id STREQUAL "debian" AND NOT lsb_id STREQUAL "ubuntu")
  message(FATAL_ERROR "We can only handle Debian or Ubuntu Deb packaging")
endif()

execute_process(
  COMMAND lsb_release --short --release
  OUTPUT_VARIABLE lsb_release
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND lsb_release --short --codename
  OUTPUT_VARIABLE CODENAME
  OUTPUT_STRIP_TRAILING_WHITESPACE
)


if(lsb_id STREQUAL "debian")
  # For Debian we want just the major release number
  string(REGEX REPLACE "\\..*" "" lsb_release "${lsb_release}")
endif()

set(ID_RELEASE "${lsb_id}${lsb_release}")

message(STATUS "OS distribution : ${ID_RELEASE}")
message(STATUS "OS code name    : ${CODENAME}")

# ----------------------------------------------------------------------
# Copy this directory and 
# ----------------------------------------------------------------------

set(DEST_DIR ${CMAKE_SOURCE_DIR}/../../debian)

file(
  COPY ${CMAKE_SOURCE_DIR}/
  DESTINATION ${DEST_DIR}
  PATTERN "*.in"   EXCLUDE
  PATTERN "CMake*" EXCLUDE
)

set(in_files
  changelog
  copyright
  rules
  mysql-connector-odbc-setup.install
  mysql-connector-odbc-test.install
  mysql-connector-odbc.install
)

foreach(_in_file ${in_files})
  configure_file(
    ${CMAKE_SOURCE_DIR}/${_in_file}.in
    ${DEST_DIR}/${_in_file}
    @ONLY
  )
endforeach()
