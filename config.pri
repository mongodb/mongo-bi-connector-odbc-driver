# #########################################################
#
# \brief	These settings are used when building 
#		Connector/ODBC using the qmake utility.
#
#               Edit these settings before using qmake to
#		generate the Makefiles.
#
# \sa		defines.pri
#
# #########################################################

# #########################################################
# Set this for config options which can be applied to all
# sources. For example you may want to turn the following
# on or off;
#
#       thread
#       warn_on
#       debug | release
#
# 'release' is included by default, to switch from command
# line to a debug build, you do
#
#   qmake CONFIG-=release CONFIG+=debug
#
# #########################################################
CONFIG		+= thread warn_on

mac {
#	CONFIG		-= thread	
} else:unix {
	INCLUDEPATH	+= $(LIBTOOL_DIR)/include
	LIBS		+= -L$(LIBTOOL_DIR)/lib
}



