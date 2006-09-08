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

# #########################################################
# UNIX
# #########################################################
unix {
        INCLUDEPATH	        += /usr/include
        LIBS                    += -L../myodbc3u -lmyodbc3u -lltdl
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
                        ../myodbc3u/MYODBCUtil.h \
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
                        ../myodbc3u/MYODBCUtilAllocDataSource.c \
                        ../myodbc3u/MYODBCUtilAllocDriver.c \          
                        ../myodbc3u/MYODBCUtilClearDataSource.c \
                        ../myodbc3u/MYODBCUtilClearDriver.c \          
                        ../myodbc3u/MYODBCUtilDefaultDataSource.c \          
                        ../myodbc3u/MYODBCUtilDSNExists.c \
                        ../myodbc3u/MYODBCUtilFreeDataSource.c \       
                        ../myodbc3u/MYODBCUtilFreeDriver.c \
                        ../myodbc3u/MYODBCUtilGetDataSourceNames.c \
                        ../myodbc3u/MYODBCUtilGetDriverNames.c \
                        ../myodbc3u/MYODBCUtilGetIniFileName.c \
                        ../myodbc3u/MYODBCUtilInsertStr.c \            
                        ../myodbc3u/MYODBCUtilReadConnectStr.c \
                        ../myodbc3u/MYODBCUtilReadDataSource.c \       
                        ../myodbc3u/MYODBCUtilReadDataSourceStr.c \
                        ../myodbc3u/MYODBCUtilReadDriver.c \           
                        ../myodbc3u/MYODBCUtilWriteConnectStr.c \
                        ../myodbc3u/MYODBCUtilWriteDataSource.c \      
                        ../myodbc3u/MYODBCUtilWriteDataSourceStr.c \
                        ../myodbc3u/MYODBCUtilWriteDriver.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgConnectAttrString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgConnectOptionString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgDiagFieldString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgEnvAttrString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgFunctionsString.c \
                        ../MYODBCDbg/MYODBCDbgLib/MYODBCDbgGetFileDefault.c \
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

