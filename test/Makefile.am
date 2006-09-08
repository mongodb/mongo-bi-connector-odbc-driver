##########################################################################
#               MAKEFILE FOR MYODBC 3.51 TEST_LIBRARY                    #
#                             UNIX                                       #
#                    (C) MySQL AB 1995-2003                              #
#                         venu@mysql.com                                 # 
##########################################################################
include Makefile.ini

noinst_PROGRAMS= \
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
my_use_result \
dbtest \
mytest \
mytest2 \
mytest3 \
mytest32

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

# PASS DSN UID PWD as the command line arguments to each sample
#

run: 
	./my_connect $(MYDSN) $(MYUID) $(MYPWD)
	./my_basics $(MYDSN) $(MYUID) $(MYPWD)
	./my_param $(MYDSN) $(MYUID) $(MYPWD)
	./my_result $(MYDSN) $(MYUID) $(MYPWD)
	./my_cursor $(MYDSN) $(MYUID) $(MYPWD)
	./my_tran $(MYDSN) $(MYUID) $(MYPWD)
	./my_position $(MYDSN) $(MYUID) $(MYPWD)
	./my_relative $(MYDSN) $(MYUID) $(MYPWD)
	./my_scroll $(MYDSN) $(MYUID) $(MYPWD)
	./my_col_length $(MYDSN) $(MYUID) $(MYPWD)
	./my_blob $(MYDSN) $(MYUID) $(MYPWD)
	./my_bulk $(MYDSN) $(MYUID) $(MYPWD)
	./my_keys $(MYDSN) $(MYUID) $(MYPWD)
	./my_unixodbc $(MYDSN) $(MYUID) $(MYPWD)
	./my_dyn_cursor $(MYDSN) $(MYUID) $(MYPWD)
	./my_tran_ext $(MYDSN) $(MYUID) $(MYPWD)
	./my_catalog $(MYDSN) $(MYUID) $(MYPWD)
	./my_error $(MYDSN) $(MYUID) $(MYPWD)
	./my_use_result 1000 $(MYDSN) $(MYUID) $(MYPWD)	

trun:
	./mytest32 $(MYDSN) $(MYUID) $(MYPWD)
	./mytest32 $(MYDSN) $(MYUID) $(MYPWD)
	./mytest3 -1 $(MYDSN) $(MYUID) $(MYPWD)
	./mytest3 -1 $(MYDSN) $(MYUID) $(MYPWD)
	./mytest -1 $(MYDSN) $(MYUID) $(MYPWD)
	./mytest -1 $(MYDSN) $(MYUID) $(MYPWD)

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
