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
DEFINES += VERSION=\"3.51.13\"

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
# Define MYODBC_DBG to enable debug trace/log. Set environment 
# variable.
#
# Set MYODBC_LOG to enable/disable at exec time.
#
#       1. undefined MYODBC_LOG         - No log
#       2. SET MYODBC_LOG=off           - No log 
#       3. SET MYODBC_LOG=stdout        - Log to stdout
#       4. SET MYODBC_LOG=stderr        - Log to stderr
#       5. SET MYODBC_LOG=<filename>    - Log appended to <filename>
#
# #########################################################
DEFINES += MYODBC_DBG

# #########################################################
# Define the ammount of output from post-build tests. The
# higher the number the more output.
# #########################################################
DEFINES += DEBUG_LEVEL=2

