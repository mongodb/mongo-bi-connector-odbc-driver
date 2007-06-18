#include <sql.h>
#include <sqlext.h>

SQLRETURN SQL_API SQLColAttribute( SQLHSTMT  StatementHandle,
                                   SQLUSMALLINT ColumnNumber,
                                   SQLUSMALLINT FieldIdentifier,
                                   SQLPOINTER  CharacterAttributePtr,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *StringLengthPtr,
                                   SQLLEN *  NumericAttributePtr )
{ return 1; }

int main() {
}
