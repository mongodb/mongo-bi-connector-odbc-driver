# #########################################################
#
# \brief        Use qt qmake on this to make the Makefile.
#
#               This file is really only for XP because UNIX, 
#               Linux, and OSX can use the gnu automake.
#
#               This file exists for those that have qt (such as
#               myself). Others may simply use the Makefile.win
#
# #########################################################

# #########################################################
# COMMON
# #########################################################
TEMPLATE                = app
TARGET                  = myodbc3m
DESTDIR                 = ../bin
include( ../common.pri )
include( ../config.pri )
CONFIG                  += console
include( ../defines.pri )
include( ../odbc.pri )

# #########################################################
# WIN
# #########################################################
win32 {
        LIBS              += ..\util\myodbc3u.lib
}

# #########################################################
# UNIX
# #########################################################
unix {
        LIBS               += -L../util -lmyodbc3u
}

# #########################################################
# OSX
# #########################################################
mac {
}

# #########################################################
# FILES
# #########################################################
HEADERS			= \
                        myodbc3m.h
SOURCES			= \
                        myodbc3m.c

