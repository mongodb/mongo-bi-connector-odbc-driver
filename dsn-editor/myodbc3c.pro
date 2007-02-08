# #########################################################
#
# \brief        This is a qmake project file for building the
#               the Qt based GUI MYODBCConfig application.
#
#               The build here is done by using the setup code
#               directly - at build-time.
#
# #########################################################

# #########################################################
# COMMON
# #########################################################
TEMPLATE                = app
TARGET                  = myodbc3c
DESTDIR                 = ../bin
include( ../common.pri )
include( ../config.pri )
CONFIG                  += qt
include( ../defines.pri )
include( ../odbc.pri )

INCLUDEPATH             += ../myodbc3u ../setup

# #########################################################
# WIN32
# #########################################################
win32 {
        LIBS            += user32.lib ..\myodbc3u\myodbc3u.lib
# RC_FILE         += MYODBCConfig.rc
}

# #########################################################
# UNIX
# #########################################################
mac {
} else:unix {
#        INCLUDEPATH     += ../myodbc3u ../setup /usr/include
        LIBS            += -L../myodbc3u -lmyodbc3u
}

# #########################################################
# OSX
#	On OSX we produce a directory called; MYODBCConfig.app
#	which contains everything needed to execute the app
#	using the 'open' command or by double-clicking in
#	Finder. MYODBCConfig.app can be copied to where desired
#	in the file system (for example; /Applications/Utilities).
# #########################################################
mac {
#        DEFINES		-= HAVE_ODBCINST_H
#        DEFINES		+= HAVE_IODBCINST_H
        LIBS            -= -lmyodbc3S
        LIBS            += -L../myodbc3u -lmyodbc3u
        LIBS            += -lltdl -framework Carbon -framework QuickTime -lz -framework OpenGL -framework AGL -lz
        RC_FILE         = MYODBCConfig.icns
}

# #########################################################
# FILES
# #########################################################
HEADERS                 += \
                        ../myodbc3u/MYODBCUtil.h \ 
                        ../setup/MYODBCSetup.h \
                        ../setup/MYODBCSetupAssistText.h \
                        ../setup/MYODBCSetupCheckBox.h \
                        ../setup/MYODBCSetupComboBox.h \
                        ../setup/MYODBCSetupComboBoxDatabases.h \
                        ../setup/MYODBCSetupDataSourceDialog.h \
                        ../setup/MYODBCSetupDataSourceTab1.h \
                        ../setup/MYODBCSetupDataSourceTab2.h \
                        ../setup/MYODBCSetupDataSourceTab3.h \
                        ../setup/MYODBCSetupDataSourceTab3a.h \
                        ../setup/MYODBCSetupDataSourceTab3b.h \
                        ../setup/MYODBCSetupDataSourceTab3c.h \
                        ../setup/MYODBCSetupDataSourceTab3d.h \
                        ../setup/MYODBCSetupLineEdit.h

SOURCES                 += \
                        main.cpp \
                        ../setup/MYODBCSetupAssistText.cpp \
                        ../setup/MYODBCSetupCheckBox.cpp \
                        ../setup/MYODBCSetupComboBox.cpp \
                        ../setup/MYODBCSetupComboBoxDatabases.cpp \
                        ../setup/MYODBCSetupConfigDSNAdd.c \
                        ../setup/MYODBCSetupConfigDSNEdit.c \
                        ../setup/MYODBCSetupConfigDSNRemove.c \
                        ../setup/MYODBCSetupDataSourceConfig.cpp \
                        ../setup/MYODBCSetupDataSourceDialog.cpp \
                        ../setup/MYODBCSetupDataSourceTab1.cpp \
                        ../setup/MYODBCSetupDataSourceTab2.cpp \
                        ../setup/MYODBCSetupDataSourceTab3.cpp \
                        ../setup/MYODBCSetupDataSourceTab3a.cpp \
                        ../setup/MYODBCSetupDataSourceTab3b.cpp \
                        ../setup/MYODBCSetupDataSourceTab3c.cpp \
                        ../setup/MYODBCSetupDataSourceTab3d.cpp \
                        ../setup/MYODBCSetupDriverConnect.c \
                        ../setup/MYODBCSetupDriverConnectPrompt.cpp \
                        ../setup/MYODBCSetupHandleInstallerError.c \
                        ../setup/MYODBCSetupLineEdit.cpp



