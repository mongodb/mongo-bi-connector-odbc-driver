#
# Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.
# Copyright (c) 2018-Present MongoDB Inc.
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

SET(CONNECTOR_MAJOR "1")
SET(CONNECTOR_MINOR "4")
SET(CONNECTOR_PATCH "6")
SET(CONNECTOR_LEVEL "")
SET(CONNECTOR_QUALITY "GA")

# ----------------------------------------------------------------------
# Set other variables that are about the version
# ----------------------------------------------------------------------

IF(CONNECTOR_MINOR LESS 10)
	SET(CONNECTOR_MINOR_PADDED "0${CONNECTOR_MINOR}")
ELSE(CONNECTOR_MINOR LESS 10)
	SET(CONNECTOR_MINOR_PADDED "${CONNECTOR_MINOR}")
ENDIF(CONNECTOR_MINOR LESS 10)

# If driver survives 100th patch this has to be changed
IF(CONNECTOR_PATCH LESS 10)
	SET(CONNECTOR_PATCH_PADDED "000${CONNECTOR_PATCH}")
ELSE(CONNECTOR_PATCH LESS 10)
	SET(CONNECTOR_PATCH_PADDED "00${CONNECTOR_PATCH}")
ENDIF(CONNECTOR_PATCH LESS 10)

SET(CONNECTOR_BASE_VERSION    "${CONNECTOR_MAJOR}.${CONNECTOR_MINOR}")
SET(CONNECTOR_BASE_PREVIOUS   "5.2")
SET(CONNECTOR_NUMERIC_VERSION "${CONNECTOR_BASE_VERSION}.${CONNECTOR_PATCH}")
SET(CONNECTOR_VERSION         "${CONNECTOR_NUMERIC_VERSION}${EXTRA_VERSION_SUFFIX}${CONNECTOR_LEVEL}")
