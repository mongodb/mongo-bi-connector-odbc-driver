# #########################################################
# COMMON
# #########################################################
TEMPLATE        = app
TARGET		= MYQTODBCCls
DESTDIR		= ../../bin
include( ../../common.pri )
include( ../../config.pri )
include( ../../defines.pri )
include( ../../odbc.pri )
CONFIG	        += qt windows
INCLUDEPATH	+= ../include ../../MYQTODBCCls/include ../../MYODBCCls/include

# #########################################################
# WIN32
# #########################################################
win32 {
        DEFINES -= UNICODE
#       RC_FILE	+= MYQTODBCCls.rc
        LIBS	+= ..\..\lib\MYQTODBCCls.lib ..\..\lib\MYODBCCls.lib
}

# #########################################################
# UNIX
# #########################################################
unix {
	LIBS	+= -L../../lib -lMYQTODBCCls -lMYODBCCls
}

# #########################################################
# OSX
# #########################################################
mac {
        RC_FILE = MYQTODBCCls.icns
}

# #########################################################
# FILES
# #########################################################
HEADERS			+= \
			MYQTODBCCls.h

SOURCES			+= \
			main.cpp \
			MYQTODBCCls.cpp

