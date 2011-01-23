#--------------------------------------------------------
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


#-------------- FIND WIX_DIR ------------------
IF(DEFINED $ENV{WIX_DIR})
  SET(WIX_DIR "$ENV{WIX_DIR}")
ENDIF(DEFINED $ENV{WIX_DIR})

# Wix installer creates WIX environment variable
FIND_PATH(WIX_DIR candle.exe
	$ENV{WIX_DIR}/bin
	$ENV{WIX}/bin
	"$ENV{ProgramFiles}/wix/bin"
	"$ENV{ProgramFiles}/Windows Installer */bin")

#----------------- FIND MYSQL_LIB_DIR -------------------
IF (WIX_DIR)
	MESSAGE(STATUS "Wix found in ${WIX_DIR}")
ELSE (WIX_DIR)
	IF ($ENV{WIX_DIR})
		MESSAGE(FATAL_ERROR "Cannot find Wix in $ENV{WIX_DIR}")
	ELSE ($ENV{WIX_DIR})
		MESSAGE(FATAL_ERROR "Cannot find Wix. Please set environment variable WIX_DIR which points to the wix installation directory")
	ENDIF ($ENV{WIX_DIR})
ENDIF (WIX_DIR)

