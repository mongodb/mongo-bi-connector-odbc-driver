#ifndef MYODBC_MYSQL_H
#define MYODBC_MYSQL_H

#define MIN_MYSQL_VERSION 40000L

#ifdef __cplusplus
extern "C"
{
#endif

#define DONT_DEFINE_VOID
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <my_list.h>
#include <m_string.h>
#include <mysqld_error.h>

#if MYSQL_VERSION_ID < MIN_MYSQL_VERSION
error "MyODBC need a newer version of the MYSQL client library to compile"
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

