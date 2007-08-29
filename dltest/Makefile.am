###################################################################
#                                                                 
# BRIEF:                                                          
#	Test program to load and check a symbol in a shared lib.
#                                                                 
# DESCRIPTION:
#       This is a utility to load a shared library and check
#       for a given symbol on it. Only a test tool, not specific
#       to ODBC or databases.
#
###################################################################

noinst_PROGRAMS = dltest
dltest_SOURCES = dltest.c
dltest_LDADD   = @LTDL_LIB@

###################################################################
#
# More files to include in source distro.
#
###################################################################

EXTRA_DIST = \
	CMakeLists.txt
