#
# Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.
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

if(NOT lsb_id STREQUAL "Debian" AND NOT lsb_id STREQUAL "Ubuntu")
  message(FATAL_ERROR "We want Debian or Ubuntu")
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

string(TOLOWER "${lsb_id}${lsb_release}" ID_RELEASE)

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

configure_file(
  ${CMAKE_SOURCE_DIR}/changelog.in
  ${DEST_DIR}/changelog
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/rules.in
  ${DEST_DIR}/rules
  @ONLY
)
