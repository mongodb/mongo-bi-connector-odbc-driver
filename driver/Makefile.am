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

AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libmyodbc5.la

INCLUDES=-I$(top_srcdir)/util -I$(top_srcdir)

###################################################################
#                                                                 #
# Driver Source files                                             #
#                                                                 #
###################################################################

libmyodbc5_la_SOURCES= \
ansi.c \
catalog.c \
connect.c \
cursor.c \
desc.c \
dll.c \
driver.c \
error.c \
execute.c \
handle.c\
info.c \
options.c \
prepare.c \
results.c \
transact.c \
unicode.c \
utility.c \
driver.h

###################################################################
#                                                                 #
# Linker and preprocessor options                                 #
#                                                                 #
###################################################################
libmyodbc5_la_CPPFLAGS       = -DNONTHREADSAFE

libmyodbc5_la_LDFLAGS       = -release @NUMERIC_VERSION@ -module 

if WITH_DEBUG
LTLIBS_DEPS = $(LTLIBS_DEBUG_DEPS)
else
LTLIBS_DEPS = ../util/libmyodbc3u.la
endif
LTLIBS_DEBUG_DEPS = \
	../util/libmyodbc3u.la

libmyodbc5_la_LIBADD         = $(LTLIBS_DEPS) @LTDL_LIB@ @MYSQL_LIB@
libmyodbc5_la_DEPENDENCIES   = $(LTLIBS_DEPS)

LDFLAGS=@EXTRA_LDFLAGS@ 

EXTRA_DIST =	error.h \
		myutil.h \
                driver.def \
                driver.rc \
		CMakeLists.txt
