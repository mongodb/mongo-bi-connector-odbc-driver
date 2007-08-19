noinst_LTLIBRARIES = libmyodbc3u.la

# base source list
libmyodbc3u_la_SOURCES = \
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

EXTRA_DIST = \
	CMakeLists.txt
