/*
  Copyright (C) 2000-2007 MySQL AB

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

#ifndef _SETUPGUI_H
#define _SETUPGUI_H

#ifdef _WIN32
# include <winsock2.h>
# include <windows.h>
#endif

#include "MYODBC_MYSQL.h"
#include "installer.h"

#include <sql.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Utility functions. */
void ShowDiagnostics(SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h);
void FreeEnvHandle(SQLHENV hEnv);
SQLRETURN Connect(SQLHDBC *hDbc, SQLHENV *hEnv, DataSource *params);
void Disconnect(SQLHDBC hDbc, SQLHENV hEnv);
unsigned long CompileOptions(DataSource *params);
void DecompileOptions(DataSource *src);

/* Max DB name len, used when retrieving database list */
#define MYODBC_DB_NAME_MAX 255

/* Callbacks */
wchar_t *mytest(HWND hwnd, DataSource *params);
BOOL mytestaccept(HWND hwnd, DataSource *params);
LIST *mygetdatabases(HWND hwnd, DataSource *params);

/**
  This is the function implemented by the platform-specific code.

  @return TRUE if user pressed OK, FALSE if cancelled or closed
*/
int ShowOdbcParamsDialog(DataSource *params, HWND ParentWnd, BOOL isPrompt);

#ifdef __cplusplus
}
#endif

#endif
