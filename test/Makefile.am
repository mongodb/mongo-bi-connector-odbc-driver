##########################################################################
#               MAKEFILE FOR MYODBC 3.51 TEST_LIBRARY                    #
#                             UNIX                                       #
#                    (C) MySQL AB 1995-2003                              #
#                         venu@mysql.com                                 # 
##########################################################################

# Set up to be able to run tests
# 
# The INI filename has to include "odbc.ini" to work around a bug in the
# version of iODBC shipped with Mac OS X.
#
TEST_ODBCINI   = test-odbc.ini
TEST_DSN       = myodbc3
TEST_DRIVER    = MySQL ODBC 3.51 Driver
TEST_DATABASE  = test
TEST_SERVER    = localhost
TEST_UID       = root
TEST_PASSWORD  = 
TEST_SOCKET    = /tmp/mysql.sock

plain_tests= \
my_connect \
my_basics \
my_param \
my_result \
my_tran \
my_cursor \
my_position \
my_relative \
my_scroll \
my_col_length \
my_blob \
my_bulk \
my_dyn_cursor \
my_unixodbc \
my_keys \
my_tran_ext \
my_catalog \
my_error \
mytest32
fancy_tests= \
mytest \
mytest2 \
mytest3
odd_tests= \
dbtest \
my_use_result

noinst_PROGRAMS= $(plain_tests) $(fancy_tests) $(odd_tests)

#	
# COMPILATION SPECIFICATION
# 

INCLUDES= 		-I$(top_srcdir)/test/include
AM_LDFLAGS= 		@myodbc_test_linklib@

my_connect_SOURCES=	connect/my_connect.c
my_basics_SOURCES=	basics/my_basics.c
my_param_SOURCES=	param/my_param.c
my_result_SOURCES=	result/my_result.c
my_tran_SOURCES=	tran/my_tran.c
my_cursor_SOURCES=	cursor/my_cursor.c
my_position_SOURCES=	position/my_position.c
my_relative_SOURCES=	relative/my_relative.c
my_scroll_SOURCES=	scroll/my_scroll.c
my_col_length_SOURCES=	col_length/my_col_length.c
my_blob_SOURCES=	blob/my_blob.c
my_bulk_SOURCES=	bulk/my_bulk.c
my_dyn_cursor_SOURCES=	dyn_cursor/my_dyn_cursor.c
my_unixodbc_SOURCES=	unixodbc/my_unixodbc.c
my_keys_SOURCES=	keys/my_keys.c
my_tran_ext_SOURCES=	tran_ext/my_tran_ext.c
my_catalog_SOURCES=	catalog/my_catalog.c
my_error_SOURCES=	error/my_error.c
my_use_result_SOURCES=	use_result/my_use_result.c
dbtest_SOURCES=		db_test/db_test.c
mytest_SOURCES=		test/mytest.c
mytest2_SOURCES=	test2/mytest2.c
mytest3_SOURCES=	test3/mytest3.c
mytest32_SOURCES=	test32/mytest32.c


#
# BUILDALL SPECIFICATION, BUILD AND RUN ALL SAMPLES
#

buildall: all
testall  : all run trun
runall: testall
test: testall
testlib: testall

# ----------------------------------------------------------------------
# Pass DSN UID PASSWORD as the command line arguments to each sample
# ----------------------------------------------------------------------

run: make-test-ini $(plain_tests) my_use_result
	for test in $(plain_tests); do \
	  echo test: $$test; \
	  echo ODBCINI=$(TEST_ODBCINI) \
	    ./$$test "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" ; \
	  ODBCINI=$(TEST_ODBCINI) \
	    ./$$test "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" || \
	    test -n "$(FORCE)" || \
	    exit 1 ; \
	done
	echo test: my_use_result
	ODBCINI=$(TEST_ODBCINI) ./my_use_result 1000 "$(TEST_DSN)" "$(TEST_UID)" \
	"$(TEST_PASSWORD)" "$(TEST_SOCKET)" || \
	test -n "$(FORCE)" || \
	exit 1

trun: make-test-ini $(fancy_tests)
	for test in $(fancy_tests); do \
	  echo test: $$test; \
	  echo ODBCINI=$(TEST_ODBCINI) \
	    ./$$test -1 "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" ; \
	  ODBCINI=$(TEST_ODBCINI) \
	    ./$$test -1 "$(TEST_DSN)" "$(TEST_UID)" "$(TEST_PASSWORD)" "$(TEST_SOCKET)" || \
	    test -n "$(FORCE)" || \
	    exit 1 ; \
	done

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
	README \
	test.ini.in \
	include/mytest3.h \
	Makefile.ini \
	test.vpj \
	test.pro \
	basics/basics.pro \
	blob/blob.pro \
	bulk/bulk.pro \
	catalog/catalog.pro \
	col_length/col_length.pro \
	connect/connect.pro \
	curext/curext.pro \
	cursor/cursor.pro \
	dyn_cursor/dyn_cursor.pro \
	error/error.pro \
	keys/keys.pro \
	param/param.pro \
	position/position.pro \
	relative/relative.pro \
	result/result.pro \
	scroll/scroll.pro \
	test32/test32.pro \
	timestamp/timestamp.pro \
	tran/tran.pro \
	tran_ext/tran_ext.pro \
	unixodbc/unixodbc.pro \
	use_result/use_result.pro \
	curext/my_curext.c \
	imyodbc/myodbc.c \
	thread/my_thread.c \
	thread_coredump/thread-coredump.c \
	thread_test/thread_test.c \
	timestamp/my_ts.c

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
	test.ini.in > $(TEST_ODBCINI)
