#ifndef _CALLBACKS_UTILS_

#define _CALLBACKS_UTILS_

#include "installer.h"

#include <sql.h>

#ifdef __cplusplus
extern "C" {
#endif

void				ShowDiagnostics		( SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h );
void				FreeEnvHandle		( SQLHENV hEnv );
void				Disconnect			( SQLHDBC hDbc, SQLHENV hEnv  );
SQLRETURN			Connect				( SQLHDBC *hDbc, SQLHENV *hEnv, DataSource *params );
unsigned long		CompileOptions		( DataSource * params );
void				DecompileOptions	( DataSource *src );

#ifdef __cplusplus
}
#endif

#endif
