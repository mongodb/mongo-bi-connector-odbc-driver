#include <sql.h>
#include <sqlext.h>

SQLRETURN SQL_API SQLParamOptions( SQLHSTMT     hstmt, 
				       SQLULEN      crow,
				       SQLULEN      *pirow )
{ return 1; }

int main() {}
