##########################################################################
# Copyright (C) 1995-2002 MySQL AB, www.mysql.com                        #
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
# @description: This is the MyODBC 3.51 driver(samples) Makefile.am      #
#                                                                        #
# @author     : MySQL AB (monty@mysql.com, venu@mysql.com)               #
# @date       : 2002-10-30                                               #
# @product    : myodbc3                                                  #
#                                                                        #
##########################################################################
noinst_PROGRAMS=my_basics my_connect my_cursor my_param my_result my_tran
INCLUDES=-I..
AM_LDFLAGS=../driver/libmyodbc3.la @MYSQL_THREADSAFE_LIB@
EXTRA_DIST=my_utility.h run-samples WIN-Makefile

# test samples
test: all
	./run-samples
