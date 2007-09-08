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
	-e 's!@''QT_PATH_ARG''@!@QT_PATH_ARG@!' \
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
