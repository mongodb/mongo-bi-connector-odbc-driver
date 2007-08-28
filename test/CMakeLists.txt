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

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/test)

# put the test exe's in test/
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/test")

ENABLE_TESTING()

FOREACH(T my_basics my_blob my_bulk my_catalog my_curext my_cursor
		my_datetime my_dyn_cursor my_error my_info my_keys my_param
		my_prepare my_relative my_result my_scroll my_tran
		my_types my_unixodbc my_use_result my_bug13766)
	ADD_EXECUTABLE(${T} ${T}.c)
	TARGET_LINK_LIBRARIES(${T} myodbc3 myodbc3u mysqlclient)
	ADD_TEST(${T} ${EXECUTABLE_OUTPUT_PATH}/${T})
ENDFOREACH(T)

