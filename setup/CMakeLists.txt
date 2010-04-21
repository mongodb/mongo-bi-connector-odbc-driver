# Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
#
# The MySQL Connector/ODBC is licensed under the terms of the
# GPL, like most MySQL Connectors. There are special exceptions
# to the terms and conditions of the GPL as it is applied to
# this software, see the FLOSS License Exception available on
# mysql.com.
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
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})

SET(setup_SRCS 
			ConfigDSN.c
			MYODBCSetupAssistText.cpp
			MYODBCSetupCheckBox.cpp
			MYODBCSetupComboBox.cpp
			MYODBCSetupComboBoxDatabases.cpp
			MYODBCSetupConfigDSNAdd.c
			MYODBCSetupConfigDSNEdit.c
			MYODBCSetupConfigDSNRemove.c
			MYODBCSetupDataSourceConfig.cpp
			MYODBCSetupDataSourceDialog.cpp
			MYODBCSetupDataSourceTab1.cpp
			MYODBCSetupDataSourceTab2.cpp
			MYODBCSetupDataSourceTab3.cpp
			MYODBCSetupDataSourceTab3a.cpp
			MYODBCSetupDataSourceTab3b.cpp
			MYODBCSetupDataSourceTab3c.cpp
			MYODBCSetupDataSourceTab3d.cpp
			MYODBCSetupDataSourceTab3e.cpp
			MYODBCSetupDriverConnect.c
			MYODBCSetupDriverConnectPrompt.cpp
			MYODBCSetupHandleInstallerError.c 
			MYODBCSetupLineEdit.cpp
      myodbc3S.rc)

SET(MOC_SRCS
    MYODBCSetupAssistText.h
    MYODBCSetupCheckBox.h
    MYODBCSetupComboBox.h
    MYODBCSetupComboBoxDatabases.h
    MYODBCSetupDataSourceDialog.h
    MYODBCSetupDataSourceTab1.h
    MYODBCSetupDataSourceTab2.h
    MYODBCSetupDataSourceTab3.h
    MYODBCSetupDataSourceTab3a.h
    MYODBCSetupDataSourceTab3b.h
    MYODBCSetupDataSourceTab3c.h
    MYODBCSetupDataSourceTab3d.h
    MYODBCSetupDataSourceTab3e.h
    MYODBCSetupLineEdit.h)

IF(WIN32)
  # Adding here header files too - that is convenient if one uses VS
	SET(setup_SRCS ${setup_SRCS} ${MOC_SRCS} myodbc3S.def)
ENDIF(WIN32)

IF(QT_WRAP_CPP)
  QT_WRAP_CPP(myodbc3S setup_SRCS ${MOC_SRCS})
ENDIF(QT_WRAP_CPP)

ADD_LIBRARY(myodbc3S SHARED ${setup_SRCS})

TARGET_LINK_LIBRARIES(myodbc3S
		myodbc3u ${ODBCLIB} ${ODBCINSTLIB} ${QT_LIBRARIES})

INSTALL(TARGETS myodbc3S DESTINATION lib)

