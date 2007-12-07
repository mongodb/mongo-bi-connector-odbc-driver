#--------------------------------------------------------
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


#-------------- FIND MYSQL_INCLUDE_DIR ------------------
IF(DEFINED $ENV{WIX_DIR})
  SET(WIX_DIR "$ENV{WIX_DIR}")
ENDIF(DEFINED $ENV{WIX_DIR})
FIND_PATH(WIX_DIR candle.exe
	$ENV{WIX_DIR}/bin
	$ENV{ProgramFiles}/wix/bin
	$ENV{ProgramFiles}/Windows Installer */bin)

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

