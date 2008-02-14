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
	MYODBCUtil.h \
	MYODBCUtilAllocDataSource.c \
	MYODBCUtilAllocDriver.c \
	MYODBCUtilClearDataSource.c \
	MYODBCUtilClearDriver.c \
	MYODBCUtilDefaultDataSource.c \
	MYODBCUtilFreeDataSource.c \
	MYODBCUtilFreeDriver.c \
	MYODBCUtilInsertStr.c \
	MYODBCUtilReadConnectStr.c \
	MYODBCUtilWriteConnectStr.c

# additional sources if supporting odbcinst
if ODBCINSTLINK
libmyodbc3u_la_SOURCES += \
	MYODBCUtilGetDataSourceNames.c \
	MYODBCUtilGetDriverNames.c \
	MYODBCUtilGetIniFileName.c \
	MYODBCUtilDSNExists.c \
	MYODBCUtilReadDataSource.c \
	MYODBCUtilReadDataSourceStr.c \
	MYODBCUtilReadDriver.c \
	MYODBCUtilWriteDataSource.c \
	MYODBCUtilWriteDataSourceStr.c \
	MYODBCUtilWriteDriver.c
endif

# libmysql lib deps
libmyodbc3u_la_LIBADD         = $(LTLIBS_DEPS) @LTDL_LIB@ @MYSQL_LIB@
libmyodbc3u_la_DEPENDENCIES   = $(LTLIBS_DEPS)

EXTRA_DIST = \
	CMakeLists.txt
