# #########################################################
# COMMON
# #########################################################
TEMPLATE	= app
TARGET		= MYODBCCls
DESTDIR		= ../../bin
include( ../../common.pri )
include( ../../config.pri )
include( ../../defines.pri )
include( ../../odbc.pri )
CONFIG	        += console
INCLUDEPATH	+= ../../ ../include

# #########################################################
# WIN32
# #########################################################
win32 {
        DEFINES		-= UNICODE
        DEFINES		+= Q_WS_WIN
        LIBS		+= ..\..\lib\MYODBCCls.lib
# win32:QMAKE_LFLAGS_WINDOWS    += "/NODEFAULTLIB:MSVCRT.lib"
# win32:QMAKE_LIBS_CONSOLE      += '/NODEFAULTLIB:MSVCRT.lib'
}

# #########################################################
# UNIX
# #########################################################
unix {
	DEFINES		+= Q_WS_X11
	LIBS		+= -L../../lib -lMYODBCCls
}

# #########################################################
# OSX
# #########################################################
mac {
        DEFINES += Q_WS_MACX
        RC_FILE = MYODBCCls.icns
}

# #########################################################
# FILES
# #########################################################
HEADERS			+= \
			MYODBCCls.h

SOURCES			+= \
			MYODBCCls.cpp


