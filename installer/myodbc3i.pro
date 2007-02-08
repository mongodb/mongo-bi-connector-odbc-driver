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
TARGET                  = myodbc3i
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
        LIBS              += user32.lib ..\util\myodbc3u.lib
}

# #########################################################
# UNIX
# #########################################################
unix {
        LIBS               += -L../util -lmyodbc3u -lltdl
}

# #########################################################
# OSX
# #########################################################
mac {
#        CONFIG		-= thread
#        DEFINES		-= HAVE_ODBCINST_H
#        DEFINES		+= HAVE_IODBCINST_H
}

# #########################################################
# FILES
# #########################################################
HEADERS			=
SOURCES			= \
                        myodbc3i.c

