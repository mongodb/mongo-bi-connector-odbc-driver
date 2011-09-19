###############################################################################
# Copyright (c) 1995, 2011, Oracle and/or its affiliates. All rights reserved #
#                                                                             #
# The MySQL Connector/ODBC is licensed under the terms of the GPLv2           #
# <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most          #
# MySQL Connectors. There are special exceptions to the terms and             #
# conditions of the GPLv2 as it is applied to this software, see the          #
# FLOSS License Exception                                                     #
# <http://www.mysql.com/about/legal/licensing/foss-exception.html>.           #
#                                                                             #
# This program is free software; you can redistribute it and/or modify        #
# it under the terms of the GNU General Public License as published           #
# by the Free Software Foundation; version 2 of the License.                  #
#                                                                             #
# This program is distributed in the hope that it will be useful, but         #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License     #
# for more details.                                                           #
#                                                                             #
# You should have received a copy of the GNU General Public License along     #
# with this program; if not, write to the Free Software Foundation, Inc.,     #
# 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA                     #
###############################################################################

##########################################################################
#                                                                        #
# Makefile.am                                                            #
#                                                                        #
# @description: This is the MyODBC 3.51 driver(scripts) Makefile.am      #
#                                                                        #
# @author     : MySQL AB (monty@mysql.com, venu@mysql.com)               #
# @date       : 2002-10-30                                               #
# @product    : myodbc3                                                  #
#                                                                        #
##########################################################################

CLEANFILES = make_binary_distribution \
             myodbc3.spec \
	     mysql-connector-odbc-@VERSION@.spec 

.sh:
	rm -f $@ $@-t
	sed \
	-e 's!@''VERSION''@!@VERSION@!' \
	-e 's!@''NUMERIC_VERSION''@!@NUMERIC_VERSION@!' \
	-e 's!@''MACHINE_TYPE''@!@MACHINE_TYPE@!' \
	-e 's!@''SYSTEM_TYPE''@!@SYSTEM_TYPE@!' \
	-e 's!@''ODBC_DM_PATH_ARG''@!@ODBC_DM_PATH_ARG@!' \
	-e 's!@''MYSQL_PATH_ARG''@!@MYSQL_PATH_ARG@!' \
	$< > $@-t
	chmod +x $@-t
	mv $@-t $@

all: 	make_binary_distribution \
	mysql-connector-odbc-@VERSION@.spec \
	makerpm

mysql-connector-odbc-@VERSION@.spec: myodbc3.spec
	rm -f $@
	cp myodbc3.spec $@
 
SUFFIXES = .sh 

EXTRA_DIST = \
	make_binary_distribution.sh \
        myodbc3.spec.sh \
        makerpm.sh \
	macosx/License.html.in \
	macosx/ReadMe.html.in \
	macosx/Welcome.html.in \
	macosx/myodbc.png \
	macosx/postflight
