/*
  Copyright (C) 1995-2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  The MySQL Connector/ODBC is licensed under the terms of the
  GPL, like most MySQL Connectors. There are special exceptions
  to the terms and conditions of the GPL as it is applied to
  this software, see the FLOSS License Exception available on
  mysql.com.
*/

#ifndef MYODBC_MYSQL_H
#define MYODBC_MYSQL_H

#define DONT_DEFINE_VOID
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <my_list.h>
#include <m_string.h>
#include <mysqld_error.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MIN_MYSQL_VERSION 40100L
#if MYSQL_VERSION_ID < MIN_MYSQL_VERSION
# error "Connector/ODBC requires v4.1 (or later) of the MySQL client library"
#endif

#ifdef THREAD
#include <my_pthread.h>
#else
#define pthread_mutex_lock(A)
#define pthread_mutex_unlock(A)
#define pthread_mutex_init(A,B)
#define pthread_mutex_destroy(A)
#endif

/* Get rid of defines from my_config.h that conflict with our myconf.h */
#ifdef VERSION
# undef VERSION
#endif
#ifdef PACKAGE
# undef PACKAGE
#endif

/*
  It doesn't matter to us what SIZEOF_LONG means to MySQL's headers, but its
  value matters a great deal to unixODBC, which calculates it differently.

  This causes problems where an application linked against unixODBC thinks
  SIZEOF_LONG == 4, and the driver was compiled thinking SIZEOF_LONG == 8,
  such as on Solaris x86_64 using Sun C 5.8.

  This stems from unixODBC's use of silly platform macros to guess
  SIZEOF_LONG instead of just using sizeof(long).
*/
#ifdef SIZEOF_LONG
# undef SIZEOF_LONG
#endif

#ifdef __cplusplus
}
#endif

#endif

