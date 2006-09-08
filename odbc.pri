# #########################################################
#
# \brief	These settings are used when building 
#		Connector/ODBC using the qmake utility.
#
#               Edit these settings before using qmake to
#		generate the Makefiles.
#
# \note         The plan is to generate this file using a 
#               qconfigure.bat/qconfigure at some point in 
#               the future.
#
# \sa		defines.pri
#               config.pri
#		mysql.qmake
#
# #########################################################

# #########################################################
# WIN32
# #########################################################
win32 {
        LIBS            += odbc32.lib odbccp32.lib
}

# #########################################################
# UNIX
# #########################################################
mac {
} else:unix {
	INCLUDEPATH	+= $(UNIXODBC_DIR)/include
	LIBS		+= -L$(UNIXODBC_DIR)/lib$(MYSQL_BITS) -lodbc -lodbcinst
}

# #########################################################
# OSX
# #########################################################
mac {
        LIBS		        += -liodbc -liodbcinst
}



