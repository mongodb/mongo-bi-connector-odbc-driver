# #########################################################
#
# \brief        This is a qt qmake project file. Use this to 
#               create a regular make file.
#
#               This file is really only for XP because UNIX, 
#               Linux, and OSX can use the gnu automake.
#
#               This file exists for those that have qt (such as
#               myself). Others may simply use the Makefile as is.
#
#               The idea is that we create a static lib which can
#               be used, in-place, by the setup library and the
#               driver.
#
#               Util does not include any Qt but Qt based code 
#               in MyODBC does depend upon Util.
#
# #########################################################

# #########################################################
# COMMON
# #########################################################

TEMPLATE                = lib
TARGET                  = myodbc3u
DESTDIR                 = ./
include( ../common.pri )
include( ../config.pri )
CONFIG                  += staticlib console
# plugin needed on sparc64 to get relocatable code in a static lib
# CONFIG                  += staticlib plugin
include( ../defines.pri )
include( ../odbc.pri )

# #########################################################
# WIN
# #########################################################
win32 {
}

# #########################################################
# UNIX
# #########################################################
unix {
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
                        MYODBCUtil.h                     
SOURCES			= \
                        MYODBCUtilAllocDataSource.c \
                        MYODBCUtilAllocDriver.c \          
                        MYODBCUtilClearDataSource.c \
                        MYODBCUtilClearDriver.c \          
                        MYODBCUtilDefaultDataSource.c \          
                        MYODBCUtilDSNExists.c \
                        MYODBCUtilFreeDataSource.c \       
                        MYODBCUtilFreeDriver.c \
                        MYODBCUtilGetDataSourceNames.c \
                        MYODBCUtilGetDriverNames.c \
                        MYODBCUtilInsertStr.c \            
                        MYODBCUtilReadConnectStr.c \
                        MYODBCUtilReadDataSource.c \       
                        MYODBCUtilReadDataSourceStr.c \
                        MYODBCUtilReadDriver.c \           
                        MYODBCUtilWriteConnectStr.c \
                        MYODBCUtilWriteDataSource.c \      
                        MYODBCUtilWriteDataSourceStr.c \
                        MYODBCUtilWriteDriver.c

