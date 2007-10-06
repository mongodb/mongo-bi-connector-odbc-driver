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
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} ../util ../setup)

SET(myodbc3c_SRCS
            main.cpp 
            ../setup/MYODBCSetupAssistText.cpp
            ../setup/MYODBCSetupCheckBox.cpp
            ../setup/MYODBCSetupComboBox.cpp
            ../setup/MYODBCSetupComboBoxDatabases.cpp
            ../setup/MYODBCSetupConfigDSNAdd.c
            ../setup/MYODBCSetupConfigDSNEdit.c
            ../setup/MYODBCSetupConfigDSNRemove.c
            ../setup/MYODBCSetupDataSourceConfig.cpp
            ../setup/MYODBCSetupDataSourceDialog.cpp
            ../setup/MYODBCSetupDataSourceTab1.cpp
            ../setup/MYODBCSetupDataSourceTab2.cpp
            ../setup/MYODBCSetupDataSourceTab3.cpp
            ../setup/MYODBCSetupDataSourceTab3a.cpp
            ../setup/MYODBCSetupDataSourceTab3b.cpp
            ../setup/MYODBCSetupDataSourceTab3c.cpp
            ../setup/MYODBCSetupDataSourceTab3d.cpp
            ../setup/MYODBCSetupDriverConnect.c
            ../setup/MYODBCSetupDriverConnectPrompt.cpp
            ../setup/MYODBCSetupHandleInstallerError.c
            ../setup/MYODBCSetupLineEdit.cpp)

SET(MOC_SRCS
    ../setup/MYODBCSetupAssistText.h
    ../setup/MYODBCSetupCheckBox.h
    ../setup/MYODBCSetupComboBox.h
    ../setup/MYODBCSetupComboBoxDatabases.h
    ../setup/MYODBCSetupDataSourceDialog.h
    ../setup/MYODBCSetupDataSourceTab1.h
    ../setup/MYODBCSetupDataSourceTab2.h
    ../setup/MYODBCSetupDataSourceTab3.h
    ../setup/MYODBCSetupDataSourceTab3a.h
    ../setup/MYODBCSetupDataSourceTab3b.h
    ../setup/MYODBCSetupDataSourceTab3c.h
    ../setup/MYODBCSetupDataSourceTab3d.h
    ../setup/MYODBCSetupLineEdit.h)

IF(QT_WRAP_CPP)
  QT_WRAP_CPP(myodbc3c myodbc3c_SRCS ${MOC_SRCS})
ENDIF(QT_WRAP_CPP)

ADD_EXECUTABLE(myodbc3c ${myodbc3c_SRCS})

IF(ODBC_WINDOWS)
  TARGET_LINK_LIBRARIES(myodbc3c user32)
ENDIF(ODBC_WINDOWS)

TARGET_LINK_LIBRARIES(myodbc3c myodbc3u ${ODBCLIB} ${ODBCINSTLIB} ${QT_LIBRARIES})
