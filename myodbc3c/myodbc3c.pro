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

INCLUDEPATH             += ../myodbc3u ../myodbc3S

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
#        INCLUDEPATH     += ../myodbc3u ../myodbc3S /usr/include
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
                        ../myodbc3S/MYODBCSetup.h \
                        ../myodbc3S/MYODBCSetupAssistText.h \
                        ../myodbc3S/MYODBCSetupCheckBox.h \
                        ../myodbc3S/MYODBCSetupComboBox.h \
                        ../myodbc3S/MYODBCSetupComboBoxDatabases.h \
                        ../myodbc3S/MYODBCSetupDataSourceDialog.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab1.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab2.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab3.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab3a.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab3b.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab3c.h \
                        ../myodbc3S/MYODBCSetupDataSourceTab3d.h \
                        ../myodbc3S/MYODBCSetupLineEdit.h

SOURCES                 += \
                        main.cpp \
                        ../myodbc3S/MYODBCSetupAssistText.cpp \
                        ../myodbc3S/MYODBCSetupCheckBox.cpp \
                        ../myodbc3S/MYODBCSetupComboBox.cpp \
                        ../myodbc3S/MYODBCSetupComboBoxDatabases.cpp \
                        ../myodbc3S/MYODBCSetupConfigDSNAdd.c \
                        ../myodbc3S/MYODBCSetupConfigDSNEdit.c \
                        ../myodbc3S/MYODBCSetupConfigDSNRemove.c \
                        ../myodbc3S/MYODBCSetupDataSourceConfig.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceDialog.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab1.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab2.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab3.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab3a.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab3b.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab3c.cpp \
                        ../myodbc3S/MYODBCSetupDataSourceTab3d.cpp \
                        ../myodbc3S/MYODBCSetupDriverConnect.c \
                        ../myodbc3S/MYODBCSetupDriverConnectPrompt.cpp \
                        ../myodbc3S/MYODBCSetupHandleInstallerError.c \
                        ../myodbc3S/MYODBCSetupLineEdit.cpp



