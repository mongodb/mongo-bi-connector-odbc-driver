# #########################################################
# COMMON
# #########################################################
TEMPLATE        = lib
TARGET          = MYODBCCls
DESTDIR         = ../../lib
include( ../../common.pri )
include( ../../config.pri )
include( ../../defines.pri )
include( ../../odbc.pri )
CONFIG          += staticlib console
INCLUDEPATH     += ../include

# #########################################################
# WIN32
# #########################################################
win32 {
        DEFINES -= UNICODE
        DEFINES += Q_WS_WIN
}

# #########################################################
# UNIX
# #########################################################
unix {
	DEFINES	+= Q_WS_X11
}

# #########################################################
# OSX
# #########################################################
mac {
        DEFINES += Q_WS_MACX
}

# #########################################################
# FILES
# #########################################################
HEADERS		= \
		../include/MYODBCConnection.h \
		../include/MYODBCEnvironment.h \
		../include/MYODBCMessage.h \
		../include/MYODBCObject.h \
		../include/MYODBCObjectList.h \
		../include/MYODBCStatement.h

SOURCES 	= \
		MYODBCConnection.cpp \
		MYODBCEnvironment.cpp \
		MYODBCMessage.cpp \
		MYODBCObject.cpp \
		MYODBCObjectList.cpp \
		MYODBCStatement.cpp
