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

# Setup to be able to run tests
# 
# The INI filename has to include "odbc.ini" to work around a bug in the
# version of iODBC shipped with Mac OS X.
#
TEST_ODBCINI   = odbc.ini
TEST_ODBCINSTINI = odbcinst.ini
TEST_DSN       = myodbc3
# XXX this is not really correct, but works for testing.
TEST_DRIVER    = ../driver/.libs/libmyodbc3.so
TEST_DATABASE  = test
TEST_SERVER    = localhost
TEST_UID       = root
TEST_PASSWORD  = 
TEST_SOCKET    = /tmp/mysql.sock

tap_tests= \
my_basics \
my_blob \
my_bulk \
my_catalog \
my_curext \
my_cursor \
my_datetime \
my_dyn_cursor \
my_error \
my_keys \
my_param \
my_position \
my_relative \
my_result \
my_scroll \
my_tran \
mytest \
mytest32

plain_tests= \
my_col_length \
my_unixodbc \
my_use_result
fancy_tests= \
mytest3

noinst_PROGRAMS= $(plain_tests) $(fancy_tests) $(tap_tests)

#	
# COMPILATION SPECIFICATION
# 

INCLUDES= 		-I$(top_srcdir)/test/include
AM_LDFLAGS= 		@myodbc_test_linklib@

my_col_length_SOURCES=	col_length/my_col_length.c
my_unixodbc_SOURCES=	unixodbc/my_unixodbc.c
my_use_result_SOURCES=	use_result/my_use_result.c
mytest3_SOURCES=	test3/mytest3.c


#
# BUILDALL SPECIFICATION, BUILD AND RUN ALL SAMPLES
#

buildall: all
testall: all run-tap run trun
runall: testall
test: testall
testlib: testall

# ----------------------------------------------------------------------
# Pass DSN UID PASSWORD as the command line arguments to each sample
# ----------------------------------------------------------------------

run: make-test-ini $(plain_tests)
	for test in $(plain_tests); do \
	  echo test: $$test; \
	  echo ODBCSYSINI=. \
	    ODBCINI=$(TEST_ODBCINI) ODBCINSTINI=$(TEST_ODBCINSTINI) \
	    ./$$test "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" ; \
	  ODBCSYSINI=. \
	    ODBCINI=$(TEST_ODBCINI) ODBCINSTINI=$(TEST_ODBCINSTINI) \
	    ./$$test "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" || \
	    test -n "$(FORCE)" || \
	    exit 1 ; \
	done

trun: make-test-ini $(fancy_tests)
	for test in $(fancy_tests); do \
	  echo test: $$test; \
	  echo ODBCSYSINI=. \
	    ODBCINI=$(TEST_ODBCINI) ODBCINSTINI=$(TEST_ODBCINSTINI) \
	    ./$$test -1 "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" ; \
	  ODBCSYSINI=. \
	    ODBCINI=$(TEST_ODBCINI) ODBCINSTINI=$(TEST_ODBCINSTINI) \
	    ./$$test -1 "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" || \
	    test -n "$(FORCE)" || \
	    exit 1 ; \
	done

run-tap: make-test-ini $(tap_tests)
	perl $(srcdir)/unit.pl run $(tap_tests)

#
# CLEAN SPECIFICATION
#

clean-local:
	rm -f my*dump
	rm -f core
	rm -f *~
	rm -f *.log
	rm -f *.da
	rm -f *.bb
	rm -f *.bbg
	rm -f gmon.*


EXTRA_DIST= \
	odbc.ini.in \
	odbcinst.ini.in \
	include/mytest3.h \
	include/odbctap.h \
	Makefile.ini \
	test.vpj \
	imyodbc/myodbc.c \
	thread/my_thread.c \
	thread_coredump/thread-coredump.c \
	thread_test/thread_test.c

# ----------------------------------------------------------------------
# We want to rebuild the "test.ini" file each time, as it might
# need an update from overriding variables on the command line
# ----------------------------------------------------------------------

.PHONY: make-test-ini

make-test-ini:
	sed \
	  -e 's!@''TEST_DSN''@!$(TEST_DSN)!g' \
	  -e 's!@''TEST_DRIVER''@!$(TEST_DRIVER)!g' \
	  -e 's!@''TEST_DATABASE''@!$(TEST_DATABASE)!g' \
	  -e 's!@''TEST_SERVER''@!$(TEST_SERVER)!g' \
	  -e 's!@''TEST_UID''@!$(TEST_UID)!g' \
	  -e 's!@''TEST_PASSWORD''@!$(TEST_PASSWORD)!g' \
	  -e 's!@''TEST_SOCKET''@!$(TEST_SOCKET)!g' \
	odbc.ini.in > $(TEST_ODBCINI)
	sed \
	  -e 's!@''TEST_DSN''@!$(TEST_DSN)!g' \
	  -e 's!@''TEST_DRIVER''@!$(TEST_DRIVER)!g' \
	  -e 's!@''TEST_DATABASE''@!$(TEST_DATABASE)!g' \
	  -e 's!@''TEST_SERVER''@!$(TEST_SERVER)!g' \
	  -e 's!@''TEST_UID''@!$(TEST_UID)!g' \
	  -e 's!@''TEST_PASSWORD''@!$(TEST_PASSWORD)!g' \
	  -e 's!@''TEST_SOCKET''@!$(TEST_SOCKET)!g' \
	odbcinst.ini.in > $(TEST_ODBCINSTINI)
