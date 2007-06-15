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

#endif
