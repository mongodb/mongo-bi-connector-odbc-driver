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
*/

#ifndef MYODBC_ODBC_H
#define MYODBC_ODBC_H

#define ODBCVER 0x0351

#ifdef _UNIX_
# include <ctype.h>
# include <ltdl.h>
# include <sql.h>
# include <sqlext.h>
# ifdef USE_IODBC
#  include <iodbcinst.h>
# else
#  include <odbcinst.h>
# endif

# ifndef SYSTEM_ODBC_INI
#  define BOTH_ODBC_INI ODBC_BOTH_DSN
#  define USER_ODBC_INI ODBC_USER_DSN
#  define SYSTEM_ODBC_INI ODBC_SYSTEM_DSN
# endif

/* If SQL_API is not defined, define it, unixODBC doesn't have this */
# if !defined(SQL_API)
#  define SQL_API
# endif
#else
# include <windows.h>
# ifndef RC_INVOKED
#  pragma pack(1)
# endif

# include <sql.h>
# include <sqlext.h>
# include <odbcinst.h>
#endif

#endif /* !MYODBC_ODBC_H */
