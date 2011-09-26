INCLUDES = -I$(top_srcdir)

noinst_LTLIBRARIES = libmyodbc3u.la

# base source list
libmyodbc3u_la_SOURCES = \
	stringutil.h \
	stringutil.c \
	installer.h \
	installer.c \
	odbcinstw.c \
	unicode_transcode.c \
	MYODBCUtil.h

# libmysql lib deps
libmyodbc3u_la_LIBADD         = $(LTLIBS_DEPS) @DL_LIB@ @MYSQL_LIB@
libmyodbc3u_la_DEPENDENCIES   = $(LTLIBS_DEPS)

EXTRA_DIST = \
	CMakeLists.txt
