# Copyright (c) 2003, 2011, Oracle and/or its affiliates. All rights reserved.
#
# The MySQL Connector/ODBC is licensed under the terms of the GPLv2
# <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
# MySQL Connectors. There are special exceptions to the terms and
# conditions of the GPLv2 as it is applied to this software, see the
# FLOSS License Exception
# <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

# Setup to be able to run tests
# 
# The INI filename has to include "odbc.ini" to work around a bug in the
# version of iODBC shipped with Mac OS X.
#
TEST_PREFIX    = 
TEST_ODBCINI   = odbc.ini
TEST_ODBCINSTINI = odbcinst.ini
TEST_DSN       = myodbc5
# XXX this is not really correct, but works for testing.
TEST_DRIVER    = ../driver/.libs/libmyodbc5.so
TEST_DATABASE  = test
TEST_SERVER    = localhost
TEST_UID       = root
TEST_PASSWORD  = 
TEST_SOCKET    = /tmp/mysql.sock
VALGRIND       =

INCLUDES= 		-I$(top_srcdir)/test
AM_LDFLAGS= 		@myodbc_test_linklib@

tap_tests= \
my_basics \
my_blob \
my_bulk \
my_catalog \
my_curext \
my_cursor \
my_datetime \
my_desc \
my_dyn_cursor \
my_error \
my_info \
my_keys \
my_param \
my_prepare \
my_relative \
my_result \
my_scroll \
my_tran \
my_types \
my_unicode \
my_unixodbc \
my_use_result \
my_bug13766

noinst_PROGRAMS= $(tap_tests)
# This is a cheat -- we use MYSQL_LIB only because we need the threading
# library flags, not the MySQL client library itself
LDADD= @MYSQL_LIB@

test: make-test-ini $(tap_tests)
	ODBCSYSINI=. \
	ODBCINI=$(TEST_ODBCINI) ODBCINSTINI=$(TEST_ODBCINSTINI) \
	TEST_DRIVER=$(TEST_DRIVER) TEST_DSN=$(TEST_DSN) TEST_UID=$(TEST_UID) \
	TEST_PASSWORD=$(TEST_PASSWORD) TEST_SOCKET=$(TEST_SOCKET) \
	perl $(srcdir)/unit.pl run $(tap_tests)

test-bt: make-test-ini $(tap_tests)
	-for f in $(tap_tests) ; \
	do \
	  echo "******************************************************************************" ; \
	  echo "Starting test $(TEST_PREFIX)$$f" ; \
	  ODBCSYSINI=. \
	  ODBCINI=$(TEST_ODBCINI) ODBCINSTINI=$(TEST_ODBCINSTINI) \
	  TEST_DRIVER=$(TEST_DRIVER) TEST_DSN=$(TEST_DSN) TEST_UID=$(TEST_UID) \
	  TEST_PASSWORD=$(TEST_PASSWORD) TEST_SOCKET=$(TEST_SOCKET) \
	  $(VALGRIND) ./$$f ; \
	  echo "Ending test $(TEST_PREFIX)$$f" ; \
	done ; \
	echo "" ; \
	echo "******************************************************************************"

EXTRA_DIST= \
	odbc.ini.in \
	odbcinst.ini.in \
	odbctap.h \
	unit.pl \
	CMakeLists.txt \
  cmake/generateinifiles.cmake

# We want to rebuild the "test.ini" file each time, as it might need an
# update from overriding variables on the command line
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
