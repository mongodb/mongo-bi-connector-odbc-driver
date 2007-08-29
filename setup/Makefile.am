###################################################################
#                                                                 
# BRIEF:                                                          
#	This is used by the GNU auto-build to create a Makefile for
#       building and installing the Qt based GUI setup library.
#                                                                 
# DESCRIPTION:
#       This is an optional part of the build. It is possible to build
#       this if the gui flag has been set in configure (default is yes)
#       and if Qt is installed.
#
###################################################################

AUTOMAKE_OPTIONS=foreign

if QT
lib_LTLIBRARIES = libmyodbc3S.la
else
lib_LTLIBRARIES = 
endif

###################################################################
#                                                                 #
# Source files                                                    #
#                                                                 #
###################################################################

libmyodbc3S_la_SOURCES = \
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

nodist_libmyodbc3S_la_SOURCES = \
	mMYODBCSetupAssistText.cpp \
	mMYODBCSetupCheckBox.cpp \
	mMYODBCSetupComboBox.cpp \
	mMYODBCSetupComboBoxDatabases.cpp \
	mMYODBCSetupDataSourceDialog.cpp \
	mMYODBCSetupDataSourceTab1.cpp \
	mMYODBCSetupDataSourceTab2.cpp \
	mMYODBCSetupDataSourceTab3.cpp \
	mMYODBCSetupDataSourceTab3a.cpp \
	mMYODBCSetupDataSourceTab3b.cpp \
	mMYODBCSetupDataSourceTab3c.cpp \
	mMYODBCSetupDataSourceTab3d.cpp \
	mMYODBCSetupLineEdit.cpp

mMYODBCSetupAssistText.cpp: @srcdir@/MYODBCSetupAssistText.h
	@MOC@ @srcdir@/MYODBCSetupAssistText.h -o mMYODBCSetupAssistText.cpp

mMYODBCSetupCheckBox.cpp: @srcdir@/MYODBCSetupCheckBox.h
	@MOC@ @srcdir@/MYODBCSetupCheckBox.h -o mMYODBCSetupCheckBox.cpp

mMYODBCSetupComboBox.cpp: @srcdir@/MYODBCSetupComboBox.h
	@MOC@ @srcdir@/MYODBCSetupComboBox.h -o mMYODBCSetupComboBox.cpp

mMYODBCSetupComboBoxDatabases.cpp: @srcdir@/MYODBCSetupComboBoxDatabases.h
	@MOC@ @srcdir@/MYODBCSetupComboBoxDatabases.h -o mMYODBCSetupComboBoxDatabases.cpp

mMYODBCSetupDataSourceDialog.cpp: @srcdir@/MYODBCSetupDataSourceDialog.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceDialog.h -o mMYODBCSetupDataSourceDialog.cpp

mMYODBCSetupDataSourceTab1.cpp: @srcdir@/MYODBCSetupDataSourceTab1.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab1.h -o mMYODBCSetupDataSourceTab1.cpp

mMYODBCSetupDataSourceTab2.cpp: @srcdir@/MYODBCSetupDataSourceTab2.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab2.h -o mMYODBCSetupDataSourceTab2.cpp

mMYODBCSetupDataSourceTab3.cpp: @srcdir@/MYODBCSetupDataSourceTab3.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab3.h -o mMYODBCSetupDataSourceTab3.cpp

mMYODBCSetupDataSourceTab3a.cpp: @srcdir@/MYODBCSetupDataSourceTab3a.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab3a.h -o mMYODBCSetupDataSourceTab3a.cpp

mMYODBCSetupDataSourceTab3b.cpp: @srcdir@/MYODBCSetupDataSourceTab3b.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab3b.h -o mMYODBCSetupDataSourceTab3b.cpp

mMYODBCSetupDataSourceTab3c.cpp: @srcdir@/MYODBCSetupDataSourceTab3c.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab3c.h -o mMYODBCSetupDataSourceTab3c.cpp

mMYODBCSetupDataSourceTab3d.cpp: @srcdir@/MYODBCSetupDataSourceTab3d.h
	@MOC@ @srcdir@/MYODBCSetupDataSourceTab3d.h -o mMYODBCSetupDataSourceTab3d.cpp

mMYODBCSetupLineEdit.cpp: @srcdir@/MYODBCSetupLineEdit.h
	@MOC@ @srcdir@/MYODBCSetupLineEdit.h -o mMYODBCSetupLineEdit.cpp

###################################################################
#                                                                 #
# Build options                                                   #
#                                                                 #
###################################################################

# INCLUDES = -I@top_srcdir@/include @QT_INCLUDES@ -DSYSTEM_FILE_PATH=\"@sysconfdir@\" -DDEFLIB_PATH=\"@libdir@\" $(INCLTDL)
INCLUDES = -I@top_srcdir@/include $(QT_CXXFLAGS) -DSYSTEM_FILE_PATH=\"@sysconfdir@\" -DDEFLIB_PATH=\"@libdir@\" $(LTDLINCL)

libmyodbc3S_la_LDFLAGS = \
	-export-dynamic \
	-export-symbols @srcdir@/myodbc3S.export \
	-release @myodbc_version@ \
	-module

# libmyodbc3S_la_LIBADD = @QT_LDFLAGS@ @LIB_QT@ ../util/libmyodbc3u.la
libmyodbc3S_la_LIBADD = $(QT_LDFLAGS) $(QT_LIBS) $(EXTRA_XLIBS) \
			../util/libmyodbc3u.la

libmyodbc3S_la_DEPENDENCIES = ../util/libmyodbc3u.la

# LDFLAGS=@EXTRA_LDFLAGS@ 

###################################################################
#                                                                 #
# More files to include in source distro.                         #
#                                                                 #
###################################################################

EXTRA_DIST = \
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
	MYODBCSetupLineEdit.h \
	myodbc3S.def \
	myodbc3S.export \
	SideImage.xpm \
	TopImage.xpm \
	MySQL-16.xpm \
	myodbc3S.rc \
	CMakeLists.txt
