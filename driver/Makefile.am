# Copyright (C) 1995-2007 MySQL AB
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# There are special exceptions to the terms and conditions of the GPL
# as it is applied to this software. View the full text of the exception
# in file LICENSE.exceptions in the top-level directory of this software
# distribution.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
  TS_DRIVER=libmyodbc3_r.la
else
  TS_DRIVER=
endif

lib_LTLIBRARIES=libmyodbc3.la $(TS_DRIVER)

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

libmyodbc3_r_la_SOURCES     = $(libmyodbc3_la_SOURCES)

###################################################################
#                                                                 #
# Linker and preprocessor options                                 #
#                                                                 #
###################################################################
libmyodbc3_la_CPPFLAGS       = -DNONTHREADSAFE

libmyodbc3_la_LDFLAGS       = -release @myodbc_version@ -module 
libmyodbc3_r_la_LDFLAGS     = -release @myodbc_version@ -module 

if WITH_DEBUG
LTLIBS_DEPS = $(LTLIBS_DEBUG_DEPS)
else
LTLIBS_DEPS = ../util/libmyodbc3u.la
endif
LTLIBS_DEBUG_DEPS = \
	../util/libmyodbc3u.la

libmyodbc3_la_LIBADD         = $(LTLIBS_DEPS) @LTDL_LIB@ @MYSQL_LIB@
libmyodbc3_la_DEPENDENCIES   = $(LTLIBS_DEPS)

libmyodbc3_r_la_LIBADD       = $(LTLIBS_DEPS) @LTDL_LIB@ @MYSQL_THREADSAFE_LIB@
libmyodbc3_r_la_DEPENDENCIES = $(LTLIBS_DEPS)

LDFLAGS=@EXTRA_LDFLAGS@ 

EXTRA_DIST =	error.h \
		myutil.h \
                myodbc3.def \
                myodbc3.rc \
		CMakeLists.txt
