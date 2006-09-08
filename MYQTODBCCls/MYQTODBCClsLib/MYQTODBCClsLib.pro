# #########################################################
# COMMON
# #########################################################
TEMPLATE	= lib
TARGET          = MYQTODBCCls
DESTDIR         = ../../lib
include( ../../common.pri )
include( ../../config.pri )
include( ../../defines.pri )
include( ../../odbc.pri )
CONFIG	        += qt staticlib
INCLUDEPATH	+= ../include ../../MYODBCCls/include

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
                        ../include/MYQTODBCConnection.h \
                        ../include/MYQTODBCEnvironment.h \
			../include/MYQTODBCLogin.h \
			../include/MYQTODBCMessageOutput.h \
			../include/MYQTODBCPrompt.h \
			../include/MYQTODBCPromptDialog.h \
			../include/MYQTODBCPromptTableItem.h \
			../include/MYQTODBCPrompts.h \
			../include/MYQTODBCStatement.h

SOURCES			+= \
                        MYQTODBCConnection.cpp \
                        MYQTODBCEnvironment.cpp \
			MYQTODBCLogin.cpp \
			MYQTODBCMessageOutput.cpp \
			MYQTODBCPrompt.cpp \
			MYQTODBCPromptDialog.cpp \
			MYQTODBCPromptTableItem.cpp \
			MYQTODBCPrompts.cpp \
			MYQTODBCStatement.cpp


