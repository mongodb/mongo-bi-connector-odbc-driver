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
TARGET                  = dltest
DESTDIR                 = ../bin
include( ../common.pri )
include( ../config.pri )
CONFIG                  -= qt
CONFIG                  += console
include( ../defines.pri )

# #########################################################
# WIN
# #########################################################
win32 {
}

# #########################################################
# UNIX
# #########################################################
unix {
        LIBS            += -lltdl
}

# #########################################################
# OSX
# #########################################################
mac {
}

# #########################################################
# FILES
# #########################################################
HEADERS			=
SOURCES			= \
                        dltest.c

