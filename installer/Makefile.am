###################################################################
#                                                                 
# BRIEF:                                                          
#	This is used by the GNU auto-build to create a Makefile for
#       building the myodbcinst command-line utility.
#                                                                 
# DESCRIPTION:
#       This is an optional part of the build. It is possible to build
#       this if the myodbcinst flag is set during configure (default 
#       is yes) and libltdl is installed on the system.
#
# NOTE:
#       This program may need to be built when creating a binary 
#       distribution because it may be used when installing the binary
#       distribution. This is the case on OSX and Linux for example.
#
###################################################################

if MYODBCINST

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/util

bin_PROGRAMS = myodbc-installer
myodbc_installer_SOURCES = myodbc3i.c
myodbc_installer_LDADD        = ../util/libmyodbc3u.la @LTDL_LIB@ @MYSQL_LIB@ $(ODBC_DM_LIB)
myodbc_installer_DEPENDENCIES = ../util/libmyodbc3u.la

# LDFLAGS=@EXTRA_LDFLAGS@

endif

###################################################################
#
# More files to include in source distro.
#
###################################################################

EXTRA_DIST = \
	CMakeLists.txt
