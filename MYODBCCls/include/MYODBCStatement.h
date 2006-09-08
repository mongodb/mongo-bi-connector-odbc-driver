/* Copyright (C) 2000-2005 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   There are special exceptions to the terms and conditions of the GPL as it
   is applied to this software. View the full text of the exception in file
   EXCEPTIONS in the directory of this software distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef ODBCSTATEMENT_H
#define ODBCSTATEMENT_H

#ifndef Q_WS_WIN
#include <unistd.h>
#endif

#include <time.h>

//
#include "MYODBCObject.h"
#include "MYODBCMessage.h"

class MYODBCConnection;

// wraps SQLHSTMT

class MYODBCStatement : public MYODBCObject
{
public:
    MYODBCStatement( MYODBCConnection *pconnection );
    virtual ~MYODBCStatement();

    // SETTERS
    virtual SQLRETURN setSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER n );
    virtual SQLRETURN setSQL_ATTR_CONCURRENCY( SQLUINTEGER n );

    // GETTERS
    // - create a result set
    virtual SQLRETURN getExecute();
    virtual SQLRETURN getExecDirect( SQLCHAR *pszStatement, SQLINTEGER nLength = SQL_NTS );
    virtual SQLRETURN getTables( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszTableType = NULL, SQLSMALLINT nLength4 = 0 );
    virtual SQLRETURN getColumns( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszColumnName = NULL, SQLSMALLINT nLength4 = 0 );
    virtual SQLRETURN getStatistics( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLUSMALLINT nUnique = SQL_INDEX_ALL, SQLUSMALLINT nReserved = SQL_QUICK );
    virtual SQLRETURN getSpecialColumns( SQLSMALLINT nIdentifierType = SQL_BEST_ROWID, SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLSMALLINT nScope = SQL_SCOPE_CURROW, SQLSMALLINT nNullable = SQL_NULLABLE );
    virtual SQLRETURN getPrimaryKeys( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0 );
    virtual SQLRETURN getForeignKeys( SQLCHAR *pszPKCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszPKSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszPKTableName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszFKCatalogName = NULL, SQLSMALLINT nLength4 = 0, SQLCHAR *pszFKSchemaName = NULL, SQLSMALLINT nLength5 = 0, SQLCHAR *pszFKTableName = NULL, SQLSMALLINT nLength6 = 0 );
    virtual SQLRETURN getProcedures( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszProcName = NULL, SQLSMALLINT nLength3 = 0 );
    virtual SQLRETURN getProcedureColumns( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszProcName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszColumnName = NULL, SQLSMALLINT nLength4 = 0 );
    virtual SQLRETURN getTypeInfo( SQLSMALLINT nDataType = SQL_ALL_TYPES );

    // - process a result set
    virtual SQLRETURN getFetch();
    virtual SQLRETURN getData( SQLUSMALLINT nColumnNumber, SQLSMALLINT nTargetType, SQLPOINTER pTargetValuePtr, SQLINTEGER nBufferLength, SQLINTEGER *pnStrLenOrIndPtr );
    virtual SQLRETURN getColAttribute( SQLUSMALLINT nColumnNumber, SQLUSMALLINT nFieldIdentifier, SQLPOINTER nCharacterAttributePtr, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLengthPtr, SQLPOINTER pnNumericAttributePtr );
    virtual SQLRETURN getNumResultCols( SQLSMALLINT *pnColumnCountPtr );
    virtual SQLRETURN getRowCount( SQLINTEGER *pnRowCountPtr );

    // attributes
    virtual SQLRETURN getSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER *pn );
    virtual SQLRETURN getSQL_ATTR_CONCURRENCY( SQLUINTEGER *pn );

    //
    virtual bool getHaveCursor() { return bHaveCursor; }
    virtual MYODBCConnection *getConnection() { return pconnection; }
    virtual SQLHSTMT getStmt() { return hStmt; }
    virtual const char *getZClassName() { return "MYODBCStatement"; }
    virtual MYODBCMessage *getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState = 0, char *pszMessage = 0, SQLINTEGER nNativeCode = 0 );
    virtual MYODBCObjectList *getMessages() { return &objectlistMessages; }
    virtual double getElapsedSeconds() { return nElapsedSeconds; }

    // DO'rs
    virtual SQLRETURN doAlloc();
    virtual SQLRETURN doFree( bool bForce = false );
    virtual SQLRETURN doCloseCursor();
    virtual SQLRETURN doPrepare( SQLCHAR *pszStatementText, SQLINTEGER nTextLength );
    virtual SQLRETURN doBindCol( SQLUSMALLINT nColumnNumber, SQLSMALLINT nTargetType, SQLPOINTER pTargetValuePtr, SQLLEN nBufferLength, SQLLEN *pnStrLen_or_Ind );
    virtual SQLRETURN doCancel();

    virtual bool doWaiting();
    virtual void doErrors( MYODBCMessage::MessageTypes nType );

protected:
    MYODBCConnection *    pconnection;            // our parent
    SQLHSTMT            hStmt;                  // NULL unless we have allocated one
    bool                bHaveCursor;            // true if we have a result set
    MYODBCObjectList      objectlistMessages;     // all our messages - good or bad
    double              nElapsedSeconds;        // seconds it took for request to complete

    // attributes
    // - we need these because we want the user to be able to set/get
    //   the values even when we do not have a SQLHSTMT
    SQLUINTEGER         nSQL_ATTR_ASYNC_ENABLE; 
    SQLUINTEGER         nSQL_ATTR_CONCURRENCY; 
    
    bool                bSQL_ATTR_ASYNC_ENABLE; 
    bool                bSQL_ATTR_CONCURRENCY; 
};

#endif



