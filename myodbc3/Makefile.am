##########################################################################
# Copyright (C) 1995-2006 MySQL AB, www.mysql.com                        #
#                                                                        #
# This program is free software; you can redistribute it and/or modify   #
# it under the terms of the GNU General Public License as published by   #
# the Free Software Foundation; either version 2 of the License, or      #
# (at your option) any later version.                                    #
#                                                                        #
# This program is distributed in the hope that it will be useful,        #
# but WITHOUT ANY WARRANTY; without even the implied warranty of         #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          #
# GNU General Public License for more details.                           #
#                                                                        #
# You should have received a copy of the GNU General Public License      #
# along with this program; if not, write to the Free Software Foundation #
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          #
##########################################################################

##########################################################################
#                                                                        #
# Makefile.am                                                            #
#                                                                        #
# @description: This is the MyODBC 3.51 driver Makefile.am               #
#                                                                        #
# @author     : MySQL AB (monty@mysql.com, venu@mysql.com)               #
# @date       : 2001-Aug-15                                              #
# @product    : myodbc3                                                  #
#                                                                        #
##########################################################################

AUTOMAKE_OPTIONS=foreign
if SEPARATE_THREAD_SAFE_DRIVER
lib_LTLIBRARIES=libmyodbc3.la libmyodbc3_r.la 
else
lib_LTLIBRARIES=libmyodbc3.la
endif

###################################################################
#                                                                 #
# Driver Source files                                             #
#                                                                 #
###################################################################

libmyodbc3_la_SOURCES= \
catalog.c \
connect.c \
cursor.c \
dll.c \
error.c \
execute.c \
handle.c\
info.c \
myodbc3.c \
options.c \
prepare.c \
results.c \
transact.c \
utility.c \
myodbc3.h

libmyodbc3_r_la_SOURCES=$(libmyodbc3_la_SOURCES)

###################################################################
#                                                                 #
# Linker options                                                  #
#                                                                 #
###################################################################
if ENABLE_BUNDLES
libmyodbc3_la_LDFLAGS   = -release @myodbc_version@ -module 
libmyodbc3_r_la_LDFLAGS = -release @myodbc_version@ -module 
else
libmyodbc3_la_LDFLAGS   = -release @myodbc_version@
libmyodbc3_r_la_LDFLAGS = -release @myodbc_version@
endif

LTLIBS_DEPS=\
	../myodbc3u/libmyodbc3u.la \
	../MYODBCDbg/MYODBCDbgLib/libMYODBCDbg.la

libmyodbc3_la_LIBADD         = $(LTLIBS_DEPS) @LTDL_LIB@ @MYSQL_LIB@
libmyodbc3_la_DEPENDENCIES   = $(LTLIBS_DEPS)

libmyodbc3_r_la_LIBADD       = $(LTLIBS_DEPS) @LTDL_LIB@ @MYSQL_THREADSAFE_LIB@
libmyodbc3_r_la_DEPENDENCIES = $(LTLIBS_DEPS)

LDFLAGS=@EXTRA_LDFLAGS@ 

EXTRA_DIST =	error.h \
		myutil.h \
                myodbc3.vpj \
                myodbc3.pro \
                myodbc3.def \
                myodbc3.rc
