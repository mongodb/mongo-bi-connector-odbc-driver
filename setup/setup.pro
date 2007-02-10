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
include( ../common.pri )
include( ../config.pri )
CONFIG                  += qt plugin
include( ../defines.pri )
include( ../odbc.pri )

# Set TARGET based upon what we pickup in config.pri. It is
# natural to make debug target myodbc3Sd but we want to keep
# the name len down to 8 chars.
release {
        TARGET  = myodbc3S
}
debug {
        TARGET  = myodbc3E
}

# #########################################################
# WIN 
# #########################################################
win32 {
        LIBS                    += user32.lib ../util/myodbc3u.lib 
#        LIBS                    += /VERBOSE:LIB
        DEF_FILE                = myodbc3S.def
        RC_FILE                 = myodbc3S.rc
        libraries.path	        = /windows/system32
        libraries.files	        = myodbc3S.dll
}

# #########################################################
# UNIX
# #########################################################
unix {
        LIBS                    += -L../util -lmyodbc3u
        libraries.path	        = /usr/lib
        libraries.files	        = libmyodbc*
}

# #########################################################
# OSX
# #########################################################
mac {
#	QMAKE_LFLAGS_PLUGIN	+= -bundle -flat_namespace -undefined suppress
        libraries.path          = /usr/lib
        libraries.files         = libmyodbc3S.dylib
}

# #########################################################
# INSTALL
# #########################################################
INSTALLS		+= libraries

# #########################################################
# FILES
# #########################################################
HEADERS			= \
			MYODBCSetupAssistText.h \
			MYODBCSetupCheckBox.h \
			MYODBCSetupComboBox.h \
			MYODBCSetupComboBoxDatabases.h \
			MYODBCSetup.h \
			MYODBCSetupDataSourceDialog.h \
			MYODBCSetupDataSourceTab1.h \
			MYODBCSetupDataSourceTab2.h \
			MYODBCSetupDataSourceTab3.h \
			MYODBCSetupDataSourceTab3a.h \
			MYODBCSetupDataSourceTab3b.h \
			MYODBCSetupDataSourceTab3c.h \
			MYODBCSetupDataSourceTab3d.h \
			MYODBCSetupLineEdit.h
SOURCES			= \
			ConfigDSN.c \
			MYODBCSetupAssistText.cpp \
			MYODBCSetupCheckBox.cpp \
			MYODBCSetupComboBox.cpp \
			MYODBCSetupComboBoxDatabases.cpp \
			MYODBCSetupConfigDSNAdd.c \
			MYODBCSetupConfigDSNEdit.c \
			MYODBCSetupConfigDSNRemove.c \
			MYODBCSetupDataSourceConfig.cpp \
			MYODBCSetupDataSourceDialog.cpp \
			MYODBCSetupDataSourceTab1.cpp \
			MYODBCSetupDataSourceTab2.cpp \
			MYODBCSetupDataSourceTab3.cpp \
			MYODBCSetupDataSourceTab3a.cpp \
			MYODBCSetupDataSourceTab3b.cpp \
			MYODBCSetupDataSourceTab3c.cpp \
			MYODBCSetupDataSourceTab3d.cpp \
			MYODBCSetupDriverConnect.c \
			MYODBCSetupDriverConnectPrompt.cpp \
			MYODBCSetupHandleInstallerError.c \
			MYODBCSetupLineEdit.cpp

win32 {
HEADERS			+= \
                        ../resource.h \
                        ../VersionInfo.h
}
			
