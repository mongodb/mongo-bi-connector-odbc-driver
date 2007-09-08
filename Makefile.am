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
INCLUDES = @MYSQL_INCLUDES@
LDFLAGS=@EXTRA_LDFLAGS@
build="mysql-connector-odbc-@VERSION@-@SYSTEM_TYPE@-@MACHINE_TYPE@"

SUBDIRS = \
	util \
	driver \
	setup \
	dsn-editor \
	installer \
	monitor \
	@myodbc_test_dir@ \
	scripts \
	dltest

DIST_SUBDIRS = \
	util \
	driver \
	setup \
	dsn-editor \
	installer \
	monitor \
	test \
	scripts \
	dltest

readmedir = $(pkgdatadir)
readme_DATA = \
	ChangeLog \
	README \
	README.debug \
	INSTALL \
	LICENSE.gpl \
	LICENSE.exceptions

#
# Create the binary distribution
#
bin-dist: all
	$(top_builddir)/scripts/make_binary_distribution

bin-tar: all
	mkdir $(build) 
	$(MAKE) install prefix=`pwd`/$(build)
	cp  $(pkgdata_DATA) $(build)/
	tar cvf $(build).tar $(build) 
	gzip -9vf $(build).tar 
	rm -rf $(build)

.PHONY: test
test:
	(cd test; $(MAKE) test)

 
#
# Create a source snapshot
#
snapshot:
	$(MAKE) dist distdir=mysql-connector-odbc-@myodbc_version@-`date +"%Y%m%d"`

# Note that directories will be copied recursively
EXTRA_DIST = \
	$(readme_DATA) \
	INSTALL.win \
	BUILD.unix \
	BUILD.win \
	resource.h \
	VersionInfo.h \
	MYODBC_CONF.h \
	MYODBC_MYSQL.h \
	MYODBC_ODBC.h \
	mysql.bmp \
	Build.sh \
	Build.bat \
	CreateBinaryMsi.bat \
	CreateBinaryZip.bat \
	CreateSourceZip.bat \
	RemoveMakefiles.sh \
	Install.bat \
	PostBuildTest.bat \
	Uninstall.bat \
	Upgrade.bat \
	CMakeLists.txt \
	cmake \
	doc

# Remove Subversion ".svn" subdirectories from source distribution
dist-hook:
	rm -rf `find $(distdir) -type d -name .svn -print`
