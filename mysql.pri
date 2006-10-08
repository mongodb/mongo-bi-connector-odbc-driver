# #########################################################
#
# \brief	These settings are used when building 
#		Connector/ODBC using the qmake utility.
#
#               Edit these settings before using qmake to
#		generate the Makefiles.
#               
#               You will want to set MYSQL_DIR environment 
#               variable to something like;
#               
#               MYSQL_DIR=C:\Program Files\MySQL\MySQL Server 5.0
#
#               Using an environment variable in this way 
#               allows us to avoid problems with file names with
#               spaces.
#
# \note         The plan is to generate this file using a 
#               configure.bat/configure.exe at some point in 
#               the future.
#
# \sa		defines.pri
#               config.pri
#
# #########################################################

# #########################################################
# WIN32
# #########################################################
win32 {
        DEFINES         += _WIN32 WIN32 _WINDOWS __WIN__
        INCLUDEPATH     += $(MYSQL_DIR)\include
        LIBS		+= -L$(MYSQL_DIR)\lib\opt mysqlclient.lib zlib.lib
}

# #########################################################
# UNIX
# #########################################################
unix {
	INCLUDEPATH	+= $(MYSQL_DIR)/include/mysql
	LIBS		+= -L$(MYSQL_DIR)/lib$(MYSQL_BITS)/mysql -lmysqlclient_r 
}



