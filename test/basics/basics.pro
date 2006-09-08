# #########################################################
# COMMON
# #########################################################
TEMPLATE	= app
TARGET		= basics
DESTDIR		= ../bin
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
}

# #########################################################
# FILES
# #########################################################
HEADERS			+= \
			 \
			..\include\mytest3.h

SOURCES			+= \
			my_basics.c

