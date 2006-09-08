# #########################################################
# COMMON
# #########################################################
TEMPLATE	= lib
TARGET          = MYODBCBrw
DESTDIR         = ../../lib
include( ../../common.pri )
include( ../../config.pri )
include( ../../defines.pri )
include( ../../odbc.pri )
CONFIG	        += qt staticlib
INCLUDEPATH	+= ../include ../../MYQTODBCCls/include ../../MYODBCCls/include

# #########################################################
# WIN32
# #########################################################
win32 {
        DEFINES -= UNICODE
}

# #########################################################
# UNIX
# #########################################################

# #########################################################
# OSX
# #########################################################

# #########################################################
# FILES
# #########################################################
HEADERS			+= \
                        ../include/MYListView.h \
                        ../include/MYListViewODBC.h \
                        ../include/MYNode.h \
                        ../include/MYNodeFolder.h \
                        ../include/MYNodeODBC.h \
                        ../include/MYNodeODBCCatalog.h \
                        ../include/MYNodeODBCColumn.h \
                        ../include/MYNodeODBCDataSource.h \
                        ../include/MYNodeODBCDataSources.h \
                        ../include/MYNodeODBCDriver.h \
                        ../include/MYNodeODBCDrivers.h \
                        ../include/MYNodeODBCEIConversion.h \
                        ../include/MYNodeODBCEIDataSource.h \
                        ../include/MYNodeODBCEIDBMS.h \
                        ../include/MYNodeODBCEIDriver.h \
                        ../include/MYNodeODBCEIFunctions.h \
                        ../include/MYNodeODBCEILimits.h \
                        ../include/MYNodeODBCEISupported.h \
                        ../include/MYNodeODBCExtendedInfo.h \
                        ../include/MYNodeODBCExtendedInfoItem.h \
                        ../include/MYNodeODBCIndex.h \
                        ../include/MYNodeODBCIndexs.h \
                        ../include/MYNodeODBCPrimaryKeys.h \
                        ../include/MYNodeODBCSchema.h \
                        ../include/MYNodeODBCSpecialColumns.h \
                        ../include/MYNodeODBCTable.h \
                        ../include/MYNodeODBCTableType.h

SOURCES			+= \
                        MYListView.cpp \
                        MYListViewODBC.cpp \
                        MYNode.cpp \
                        MYNodeFolder.cpp \
                        MYNodeODBC.cpp \
                        MYNodeODBCCatalog.cpp \
                        MYNodeODBCColumn.cpp \
                        MYNodeODBCDataSource.cpp \
                        MYNodeODBCDataSources.cpp \
                        MYNodeODBCDriver.cpp \
                        MYNodeODBCDrivers.cpp \
                        MYNodeODBCEIConversion.cpp \
                        MYNodeODBCEIDataSource.cpp \
                        MYNodeODBCEIDBMS.cpp \
                        MYNodeODBCEIDriver.cpp \
                        MYNodeODBCEIFunctions.cpp \
                        MYNodeODBCEILimits.cpp \
                        MYNodeODBCEISupported.cpp \
                        MYNodeODBCExtendedInfo.cpp \
                        MYNodeODBCExtendedInfoItem.cpp \
                        MYNodeODBCIndex.cpp \
                        MYNodeODBCIndexs.cpp \
                        MYNodeODBCPrimaryKeys.cpp \
                        MYNodeODBCSchema.cpp \
                        MYNodeODBCSpecialColumns.cpp \
                        MYNodeODBCTable.cpp \
                        MYNodeODBCTableType.cpp


