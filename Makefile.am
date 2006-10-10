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
INCLUDES = @MYSQL_INCLUDES@
LDFLAGS=@EXTRA_LDFLAGS@
build="mysql-connector-odbc-@VERSION@-@SYSTEM_TYPE@-@MACHINE_TYPE@"

SUBDIRS = \
	@myodbc_dbg_dir@ \
	myodbc3u \
	myodbc3 \
	myodbc3S \
	myodbc3c \
	myodbc3i \
	myodbc3m \
	@myodbc_samples_dir@ \
	@myodbc_test_dir@ \
	scripts \
	dltest

DIST_SUBDIRS = \
	MYODBCDbg \
	myodbc3u \
	myodbc3 \
	myodbc3S \
	myodbc3c \
	myodbc3i \
	myodbc3m \
	samples \
	test \
	scripts \
	dltest

readmedir = $(pkgdatadir)
readme_DATA = \
	ChangeLog \
	COPYING \
	EXCEPTIONS \
	README \
	README.debug

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

test:	samples testlib

#
# Run the samples
#
samples: all
	(cd samples; $(MAKE) test)

#
# Run the test library
#
testlib: all
	(cd test; $(MAKE) testall)

 
#
# Create a source snapshot
#
snapshot:
	$(MAKE) dist distdir=mysql-connector-odbc-@myodbc_version@-`date +"%Y%m%d"`

# Note that directories will be copied recursively
EXTRA_DIST = \
	$(readme_DATA) \
	LICENSE.gpl \
	LICENSE.exceptions \
	INSTALL \
	INSTALL.win \
	BUILD.unix \
	BUILD.osx \
	BUILD.win \
	root.pro \
	common.pri \
	config.pri \
	defines.pri \
	mysql.pri \
	odbc.pri \
	resource.h \
	VersionInfo.h \
	MYODBC_CONF.h \
	MYODBC_MYSQL.h \
	MYODBC_ODBC.h \
	mysql.bmp \
	Build.sh \
	Build.bat \
	CreateMakefiles.sh \
	CreateBinaryMsi.bat \
	CreateBinaryZip.bat \
	CreateMakefiles.bat \
	CreateSourceZip.bat \
	CreateVisualStudioProjects.bat \
	connector-odbc3.vpj \
	connector-odbc3.vpw \
	RemoveMakefiles.sh \
	Install.bat \
	PostBuildTest.bat \
	Uninstall.bat \
	Upgrade.bat \
	dltest \
	doc \
	MYODBCBrw \
	MYODBCCls \
	MYQTODBCCls

# Remove Subversion ".svn" subdirectories from source distribution
dist-hook:
	rm -rf `find $(distdir) -type d -name .svn -print`
