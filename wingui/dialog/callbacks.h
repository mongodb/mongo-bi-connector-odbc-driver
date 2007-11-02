#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#ifdef _WIN32
#include <windows.h>
#endif

#include "MYODBC_MYSQL.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Max DB name len, used when retrieving database list */
#define MYODBC_DB_NAME_MAX 255

wchar_t *mytest(HWND hwnd, DataSource* params);
BOOL mytestaccept(HWND hwnd, DataSource* params);
LIST *mygetdatabases(HWND hwnd, DataSource* params);
void myhelp(HWND hwnd);

#ifdef __cplusplus
}
#endif

#endif
