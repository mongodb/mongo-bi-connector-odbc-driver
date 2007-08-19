###################################################################
#                                                                 
# BRIEF:                                                          
#	This is used by the GNU auto-build to create a Makefile.
#                                                                 
# DESCRIPTION:
#
###################################################################

if IMYODBC

bin_PROGRAMS = myodbc3m
myodbc3m_SOURCES = myodbc3m.c
myodbc3m_LDADD=@LIBS@ @ODBC_DM_LIB@ 

# LDFLAGS=@EXTRA_LDFLAGS@

endif

###################################################################
# 
# More files to include in source distro. 
#
###################################################################

EXTRA_DIST = \
	myodbc3m.h \
	CMakeLists.txt
