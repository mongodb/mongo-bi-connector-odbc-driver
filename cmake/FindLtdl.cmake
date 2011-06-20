# Copyright (c) 2011, Oracle and/or its affiliates. All rights reserved
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


IF(LTDL_PATH)

  SET(LTDL_INCLUDES, LTDL_PATH/include)
  SET(LTDL_LFLAGS, "-L${LTDL_PATH/lib}")

  IF(EXISTS "${LTDL_PATH}/lib/libltda.la")
    SET(LTDL_LFLAGS, "-l${LTDL_PATH}/lib/libltda.la")
  ELSE(EXISTS "${LTDL_PATH}/lib/libltda.la")
    SET(LTDL_LFLAGS, "${LTDL_LFLAGS} -lltdl")
  ENDIF(EXISTS "${LTDL_PATH}/lib/libltda.la")

ELSE(LTDL_PATH)

  INCLUDE (CheckIncludeFiles)
  CHECK_INCLUDE_FILES(ltdl.h HAVE_LTDL_H)

  IF(NOT HAVE_LTDL_H)
    MESSAGE(FATAL_ERROR "ltdl.h could not be found")
  ENDIF(NOT HAVE_LTDL_H)

  INCLUDE (CheckLibraryExists)
  CHECK_LIBRARY_EXISTS(ltdl lt_dlopen "" HAVE_LTDL_LIB)

  IF(NOT HAVE_LTDL_LIB)
    MESSAGE(FATAL_ERROR "ltdl lib could not be found")
  ENDIF(NOT HAVE_LTDL_LIB)

  SET(LTDL_LFLAGS, "-lltdl")

ENDIF(LTDL_PATH)

