# #########################################################
#
# \brief	These settings are used when building 
#		Connector/ODBC using the qmake utility.
#
#               Edit these settings before using qmake to
#		generate the Makefiles.
#
# \sa		config.pri
#
# #########################################################

DEFINES += SETUP_VERSION=\"3.51.13\"

win32 {
        DEFINES -= UNICODE
}

#
# LDFLAGS was brought in to get -ldl for Solaris 8 build
#
unix {
	DEFINES += _UNIX_
	LIBS	+= $(LDFLAGS)
}

# #########################################################
# Define DBUG_OFF to leave trace code out of the build. The
# default is to include the trace code.
# #########################################################
DEFINES += DBUG_OFF 

# #########################################################
# Define the ammount of output from post-build tests. The
# higher the number the more output.
# #########################################################
DEFINES += DEBUG_LEVEL=2

