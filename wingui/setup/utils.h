#ifndef _CALLBACKS_UTILS_

#define _CALLBACKS_UTILS_

#include "../odbcdialogparams/odbcdialogparams.h"
#include "../util/installer.h"

#include <sqltypes.h>

void				ShowDiagnostics		( SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h );
void				FreeEnvHandle		( SQLHENV &hEnv );
void				Disconnect			( SQLHDBC &hDbc, SQLHENV &hEnv  );
void				Disconnect			( SQLHSTMT &hStmt, SQLHDBC &hDbc, SQLHENV &hEnv  );
const myString &	buildConnectString	( DataSource* params );
SQLRETURN			Connect				( SQLHDBC  &   hDbc, SQLHENV   &  hEnv, DataSource * params );
unsigned long		CompileOptions		( DataSource * params );
void				DecompileOptions	( DataSource &src );

#endif
