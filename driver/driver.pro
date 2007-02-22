# #########################################################
#
# \brief        Use qt qmake on this to make the Makefile.
#
#               This setup code is based upon the Qt class library
#               where the usual way to build code is to use the
#               Qt qmake tool to create a Makefile from a project
#               (this file).
#
#               On UNIX, Linux and OSX your preference should be
#               to use the GNU auto-build but this will do as an 
#               alternative.
#
#	        We need to build as a 'plugin' and not just a 'dll'
#	        because on OSX plugins are special share libraries
#	        called 'bundles' which can be explicitly loaded.
#
#	        The target directory is hard-coded but could be 
#	        done using an environment variable substitution
#	        - or one could just edit this file. The hard coding
#	        has been done just to save some time (for me).
#
# #########################################################

# #########################################################
# COMMON
# #########################################################
TEMPLATE                = lib
DESTDIR                 = ../lib
!win32:VERSION		= 3.51.13
include( ../common.pri )
include( ../config.pri )
CONFIG                  += dll
# CONFIG                  += plugin
include( ../defines.pri )
include( ../odbc.pri )
include( ../mysql.pri )

# Set TARGET based upon what we pickup in config.pri
release {
        TARGET = myodbc3
}
debug {
        TARGET = myodbc3d
}

# #########################################################
# WIN 
# We alter qmake cflags so as to use same libs as mysqlclient. We
# also bring in the util sources so as to be same :)
# #########################################################
win32 {
        debug {
                QMAKE_CFLAGS_DEBUG    -= -MDd
                QMAKE_CFLAGS_DEBUG    += -MTd
        }
        release {
                QMAKE_CFLAGS_RELEASE    -= -MD
                QMAKE_CFLAGS_RELEASE    += -MT
        }

        DEFINES                 += __WIN__ ENGLISH MYODBC_EXPORTS _USERDLL DONT_DEFINE_VOID 
        LIBS                    += user32.lib advapi32.lib wsock32.lib
        DEF_FILE                = myodbc3.def
        RC_FILE                 = myodbc3.rc
        libraries.path	        = /windows/system32
        libraries.files	        = myodbc3*.dll myodbc3*.lib
}

win32-msvc2005 {
    QMAKE_LFLAGS += /manifest
}

# #########################################################
# UNIX
# #########################################################
unix {
        INCLUDEPATH	        += /usr/include
        LIBS                    += -L../util -lmyodbc3u -lltdl
        libraries.path	        = /usr/lib
        libraries.files	        = libmyodbc*
}

# #########################################################
# OSX
# #########################################################
mac {
#	QMAKE_LFLAGS_PLUGIN	+= -bundle -flat_namespace -undefined suppress
        libraries.path          = /usr/lib
        libraries.files         = libmyodbc3.dylib
}

# #########################################################
# INSTALL
# #########################################################
INSTALLS		+= libraries

# #########################################################
# FILES
# #########################################################
HEADERS			= \
                        myodbc3.h \
                        error.h \
			myutil.h \
                        ../util/MYODBCUtil.h \
		        ../MYODBCDbg/include/MYODBCDbg.h
SOURCES			= \
			catalog.c \
                        connect.c \
                        cursor.c \
                        dll.c \
                        error.c \
                        execute.c \
                        handle.c \
                        info.c \
                        myodbc3.c \
                        options.c \
                        prepare.c \
                        results.c \
                        transact.c \
                        utility.c \
                        ../util/MYODBCUtilAllocDataSource.c \
                        ../util/MYODBCUtilAllocDriver.c \          
                        ../util/MYODBCUtilClearDataSource.c \
                        ../util/MYODBCUtilClearDriver.c \          
                        ../util/MYODBCUtilDefaultDataSource.c \          
                        ../util/MYODBCUtilDSNExists.c \
                        ../util/MYODBCUtilFreeDataSource.c \       
                        ../util/MYODBCUtilFreeDriver.c \
                        ../util/MYODBCUtilGetDataSourceNames.c \
                        ../util/MYODBCUtilGetDriverNames.c \
                        ../util/MYODBCUtilGetIniFileName.c \
                        ../util/MYODBCUtilInsertStr.c \            
                        ../util/MYODBCUtilReadConnectStr.c \
                        ../util/MYODBCUtilReadDataSource.c \       
                        ../util/MYODBCUtilReadDataSourceStr.c \
                        ../util/MYODBCUtilReadDriver.c \           
                        ../util/MYODBCUtilWriteConnectStr.c \
                        ../util/MYODBCUtilWriteDataSource.c \      
                        ../util/MYODBCUtilWriteDataSourceStr.c \
                        ../util/MYODBCUtilWriteDriver.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgConnectAttrString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgConnectOptionString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgDiagFieldString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgEnvAttrString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgFunctionsString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgHandleTypeString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgInfoTypeString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgPosTypeString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgReturnString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgStmtAttrString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgStmtOptionString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgStmtTypeString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgTransactionTypeString.c

win32 {
HEADERS			+= \
                        ../resource.h \
                        ../VersionInfo.h
}

