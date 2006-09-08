#ifndef MYODBC_MYSQL_H
#define MYODBC_MYSQL_H

#define MIN_MYSQL_VERSION 40000

#ifdef __cplusplus
extern "C"
{
#endif

#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <my_list.h>
#include <m_string.h>

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

#ifdef __cplusplus
}
#endif

#endif

