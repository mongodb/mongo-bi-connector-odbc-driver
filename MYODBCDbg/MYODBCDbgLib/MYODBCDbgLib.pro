# #########################################################
# COMMON
# #########################################################
TEMPLATE	= lib
TARGET		= MYODBCDbg
DESTDIR		= ./
include( ../../common.pri )
include( ../../config.pri )
CONFIG          += staticlib console
include( ../../defines.pri )
include( ../../odbc.pri )
include( ../../mysql.pri )

# #########################################################
# UNIX
#       Your preference should be to use the GNU auto-build
#       on unix - but this will do as alternative.
#
#	64 bit linux
#
#	Static libs we link into a share lib - must have pos
#	independent code (PIC) so we need to add PIC to
#	CFLAGS and CXXFLAGS.
# #########################################################
unix {
        QMAKE_CFLAGS    += $$QMAKE_CFLAGS_SHLIB
        QMAKE_CXXFLAGS 	+= $$QMAKE_CXXFLAGS_SHLIB
}

# #########################################################
# FILES
# #########################################################
HEADERS		= \
		../include/MYODBCDbg.h 

SOURCES		= \
                MYODBCDbgConnectAttrString.c \
                MYODBCDbgConnectOptionString.c \
                MYODBCDbgDiagFieldString.c \
                MYODBCDbgEnvAttrString.c \
                MYODBCDbgFunctionsString.c \
                MYODBCDbgHandleTypeString.c \
                MYODBCDbgInfoTypeString.c \
                MYODBCDbgPosTypeString.c \
                MYODBCDbgReturnString.c \
                MYODBCDbgStmtAttrString.c \
                MYODBCDbgStmtOptionString.c \
                MYODBCDbgStmtTypeString.c \
                MYODBCDbgTransactionTypeString.c



