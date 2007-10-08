#pragma once

#ifndef _CALLBACKS_UTILS_

#define _CALLBACKS_UTILS_

#include "../odbcdialogparams/odbcdialogparams.h"
#include "../util/installer.h"

#include <sqltypes.h>

void				ShowDiagnostics		( SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h );
void				FreeEnvHandle		( SQLHENV &hEnv );
void				Disconnect			( SQLHDBC &hDbc, SQLHENV &hEnv  );
void				Disconnect			( SQLHSTMT &hStmt, SQLHDBC &hDbc, SQLHENV &hEnv  );
const myString &	buildConnectString	( OdbcDialogParams* params );
SQLRETURN			Connect				( SQLHDBC  &   hDbc, SQLHENV   &  hEnv, OdbcDialogParams * params );
unsigned long		CompileOptions		( OdbcDialogParams * params );
void				DS2DialogParams		( DataSource &src, OdbcDialogParams &target );
void				DialogParams2DS		( OdbcDialogParams &src, DataSource &target );

#endif
