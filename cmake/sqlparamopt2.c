#include <sql.h>
#include <sqlext.h>

SQLRETURN SQL_API SQLParamOptions( SQLHSTMT     hstmt, 
				       SQLUINTEGER      crow,
				       SQLUINTEGER      *pirow )
{ return 1; }

int main() {}
