# Copyright (c) 2006, 2016, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

SET_PROPERTY(DIRECTORY PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/sys)

SET(SYS_SOURCES array.c charset-def.c charset.c errors.c list.c
                  mf_dirname.c mf_pack.c mf_qsort.c my_access.c my_alloc.c my_div.c
                  my_error.c my_fstream.c my_getwd.c my_init.c my_lib.c my_malloc.c my_mess.c
                  my_once.c my_open.c my_read.c my_static.c my_thread.c my_thr_init.c
                  psi_noop.c sql_chars.c string.c thr_cond.c thr_mutex.c )

IF (WIN32)
  LIST(APPEND SYS_SOURCES
    my_winerr.c
    my_winfile.c
  )
ENDIF()


ADD_LIBRARY(mysql_sys ${SYS_SOURCES})

IF(NOT WIN32)
  SET_TARGET_PROPERTIES(mysql_sys PROPERTIES COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
ENDIF()

#TARGET_LINK_LIBRARIES(sys dbug strings ${ZLIB_LIBRARY} 
# ${LIBNSL} ${LIBM} ${LIBRT} ${LIBATOMIC} ${LIBEXECINFO})

# Need explicit pthread for gcc -fsanitize=address
IF(CMAKE_USE_PTHREADS_INIT AND CMAKE_C_FLAGS MATCHES "-fsanitize=")
  TARGET_LINK_LIBRARIES(sys pthread)
ENDIF()

