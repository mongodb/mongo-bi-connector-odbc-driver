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

#include "MYODBCStatement.h"

#include "MYODBCEnvironment.h"
#include "MYODBCConnection.h"

/*!
    Constructs an MYODBCStatement.
    
    It would seem natural to call doAlloc() in here so as to have a ready
    to use SQLHSTMT. However; ODBC says that SQLHDBC will not have any 
    allocated SQLHSTMT's while disconnected. So we 'could' call doAlloc()
    here if we are connected and then have MYODBCConnection call our 
    doFree() and doAlloc() as it connects/disconnects. In theory this 
    should work. In practice, Hmmm.    
*/
MYODBCStatement::MYODBCStatement( MYODBCConnection *pconnection )
    : MYODBCObject( pconnection )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    this->pconnection   = pconnection;
    hStmt               = SQL_NULL_HSTMT;
    bHaveCursor         = false;
    nElapsedSeconds     = 0;
    objectlistMessages.setAutoDelete( true );

    bSQL_ATTR_ASYNC_ENABLE  = false; 
    bSQL_ATTR_CONCURRENCY   = false; 

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYODBCStatement::~MYODBCStatement()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    // clear messages
    // - auto delete should be On... so objects will be deleted
    objectlistMessages.doClear();

    //
    if ( hStmt != SQL_NULL_HSTMT )
        doFree( true );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    An SQLUINTEGER value that specifies whether a function called with a statement on the 
    specified connection is executed asynchronously:
    
    SQL_ASYNC_ENABLE_OFF = Off (the default)
    SQL_ASYNC_ENABLE_ON = On
    
    Setting SQL_ASYNC_ENABLE_ON enables asynchronous execution for all future statement 
    handles allocated on this connection. It is driver-defined whether this enables 
    asynchronous execution for existing statement handles associated with this connection. 
    An error is returned if asynchronous execution is enabled while there is an active 
    statement on the connection.
*/
SQLRETURN MYODBCStatement::setSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER n )
{
    // SANITY CHECKS
    if ( n != SQL_ASYNC_ENABLE_OFF && n != SQL_ASYNC_ENABLE_ON )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setSQL_ATTR_ASYNC_ENABLE(): Invalid argument", -1 ) );
        return SQL_ERROR;
    }

    if ( hStmt == SQL_NULL_HSTMT )
    {
        nSQL_ATTR_ASYNC_ENABLE = n;
        bSQL_ATTR_ASYNC_ENABLE = true;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] setSQL_ATTR_ASYNC_ENABLE(): Call deferred until handle allocated", -1 ) );
        return SQL_SUCCESS;
    }

    // is asynch supported
/*
    SQLUINTEGER nAsyncMode = SQL_AM_NONE;

    if ( SQL_SUCCEEDED( pconnection->getSQL_ASYNC_MODE( &nAsyncMode ) ) )
    {
        if ( nAsyncMode == SQL_AM_CONNECTION )
        {
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setSQL_ATTR_ASYNC_ENABLE(): Asynch not supported in statement - try connection", -1 ) );
        }
        else if ( nAsyncMode == SQL_AM_STATEMENT )
        {
        }
        else if ( nAsyncMode == SQL_AM_NONE )
        {
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setSQL_ATTR_ASYNC_ENABLE(): Asynch not supported", -1 ) );
        }
    }
*/

    // DO IT!
    SQLRETURN nReturn = SQLSetStmtAttr( hStmt, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)n, SQL_IS_UINTEGER );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] SQLSetConnectAttr(): Failed to set SQL_ATTR_ASYNC_ENABLE", -1 ) );
        doErrors( MYODBCMessage::MessageError );
        return nReturn;
    }

    nSQL_ATTR_ASYNC_ENABLE = n;
    bSQL_ATTR_ASYNC_ENABLE = true;

    return nReturn;
}

/*!

*/
SQLRETURN MYODBCStatement::setSQL_ATTR_CONCURRENCY( SQLUINTEGER n )
{
    // SANITY CHECKS
    if ( hStmt == SQL_NULL_HSTMT )
    {
        nSQL_ATTR_CONCURRENCY = n;
        bSQL_ATTR_CONCURRENCY = true;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] setSQL_ATTR_CONCURRENCY(): Call deferred until handle allocated", -1 ) );
        return SQL_SUCCESS;
    }


    // DO IT!
    SQLRETURN nReturn = SQLSetStmtAttr( hStmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)n, SQL_IS_UINTEGER );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] SQLSetConnectAttr(): Failed to set SQL_ATTR_CONCURRENCY", -1 ) );
        doErrors( MYODBCMessage::MessageError );
        return nReturn;
    }

    nSQL_ATTR_CONCURRENCY = n;
    bSQL_ATTR_CONCURRENCY = true;

    return nReturn;
}

/*!
    Returns SQL_ATTR_ASYNC_ENABLE. 
    
    See setSQL_ATTR_ASYNC_ENABLE() for details.
*/
SQLRETURN MYODBCStatement::getSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER *pn )
{
    // SANITY CHECKS
    if ( !pn )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_ASYNC_ENABLE(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    if ( hStmt == SQL_NULL_HSTMT )
    {
        *pn = nSQL_ATTR_ASYNC_ENABLE;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_ASYNC_ENABLE(): Call deferred until handle allocated", -1 ) );
        return SQL_SUCCESS;
    }

    // DO IT
    SQLUINTEGER nValue;
    SQLINTEGER  nRetSize;

    SQLRETURN nReturn = SQLGetStmtAttr( hStmt, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)(&nValue), SQL_IS_UINTEGER, &nRetSize  );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] SQLGetConnectAttr(): Failed to get SQL_ATTR_ASYNC_ENABLE", -1 ) );
        doErrors( MYODBCMessage::MessageError );
        return nReturn;
    }

    // synch - for safety
    nSQL_ATTR_ASYNC_ENABLE  = nValue;
    *pn                     = nValue;

    return nReturn;
}

/*!

  
*/
SQLRETURN MYODBCStatement::getSQL_ATTR_CONCURRENCY( SQLUINTEGER *pn )
{
    // SANITY CHECKS
    if ( !pn )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_CONCURRENCY(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    if ( hStmt == SQL_NULL_HSTMT )
    {
        *pn = nSQL_ATTR_CONCURRENCY;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_CONCURRENCY(): Call deferred until handle allocated", -1 ) );
        return SQL_SUCCESS;
    }

    // DO IT
    SQLUINTEGER nValue;
    SQLINTEGER  nRetSize;

    SQLRETURN nReturn = SQLGetStmtAttr( hStmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)(&nValue), SQL_IS_UINTEGER, &nRetSize  );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] SQLGetConnectAttr(): Failed to get SQL_ATTR_CONCURRENCY", -1 ) );
        doErrors( MYODBCMessage::MessageError );
        return nReturn;
    }

    // synch - for safety
    nSQL_ATTR_CONCURRENCY  = nValue;
    *pn                    = nValue;

    return nReturn;
}

/*!
    SQLExecute
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLExecute executes a prepared statement, using the current values of the parameter marker 
    variables if any parameter markers exist in the statement.
*/
SQLRETURN MYODBCStatement::getExecute()
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;

    // - doPrepare() should have been called else SQLExecute() will catch

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLExecute( hStmt );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_NEED_DATA:
            case SQL_NO_DATA:
                return nReturn;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    } // while

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLExecDirect
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92

    SQLExecDirect executes a preparable statement, using the current values of 
    the parameter marker variables if any parameters exist in the statement. 
    SQLExecDirect is the fastest way to submit an SQL statement for one-time 
    execution.
*/
SQLRETURN MYODBCStatement::getExecDirect( SQLCHAR *pszStatement, SQLINTEGER nLength )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLExecDirect( hStmt, pszStatement, nLength );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_NO_DATA:
            case SQL_NEED_DATA:
                return nReturn;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLTables
    Version Introduced: ODBC 1.0
    Standards Compliance: X/Open
    
    SQLTables returns the list of table, catalog, or schema names, and table types, stored in a 
    specific data source. The driver returns the information as a result set.
*/
SQLRETURN MYODBCStatement::getTables( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLCHAR *pszTableType, SQLSMALLINT nLength4 )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;

    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
// printf( "[PAH][%s][%d] (%s) (%s) (%s) (%s)\n", __FILE__, __LINE__, pszCatalogName, pszSchemaName, pszTableName, pszTableType );
        nReturn = SQLTables( hStmt, pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, pszTableType, nLength4 );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLColumns
    Version Introduced: ODBC 1.0
    Standards Compliance: X/Open
    
    SQLColumns returns the list of column names in specified tables. The driver 
    returns this information as a result set on the specified StatementHandle.
*/
SQLRETURN MYODBCStatement::getColumns( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLCHAR *pszColumnName, SQLSMALLINT nLength4 )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLColumns( hStmt, pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, pszColumnName, nLength4 );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLStatistics
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLStatistics retrieves a list of statistics about a single table and the indexes associated 
    with the table. The driver returns the information as a result set.
*/
SQLRETURN MYODBCStatement::getStatistics( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLUSMALLINT nUnique, SQLUSMALLINT nReserved )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLStatistics( hStmt, pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, nUnique, nReserved );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLSpecialColumns
    Version Introduced: ODBC 1.0
    Standards Compliance: X/Open
    
    SQLSpecialColumns retrieves the following information about columns within a specified table:
    
        * The optimal set of columns that uniquely identifies a row in the table.
        * Columns that are automatically updated when any value in the row is updated by a transaction.
*/
SQLRETURN MYODBCStatement::getSpecialColumns( SQLSMALLINT nIdentifierType, SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLSMALLINT nScope, SQLSMALLINT nNullable )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLSpecialColumns( hStmt, nIdentifierType, pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, nScope, nNullable );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLPrimaryKeys
    Version Introduced: ODBC 1.0
    Standards Compliance: ODBC    
    
    SQLPrimaryKeys returns the column names that make up the primary key for a table. The driver returns 
    the information as a result set. This function does not support returning primary keys from multiple 
    tables in a single call.
*/
SQLRETURN MYODBCStatement::getPrimaryKeys( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3 )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLPrimaryKeys( hStmt, pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3 );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_INVALID_HANDLE:
            case SQL_ERROR:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }

    } // while

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLForeignKeys
    Version Introduced: ODBC 1.0
    Standards Compliance: ODBC
    
    SQLForeignKeys can return:
    
        * A list of foreign keys in the specified table (columns in the specified table that refer to primary keys in other tables).
        * A list of foreign keys in other tables that refer to the primary key in the specified table.
    
    The driver returns each list as a result set on the specified statement.
*/
SQLRETURN MYODBCStatement::getForeignKeys( SQLCHAR *pszPKCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszPKSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszPKTableName, SQLSMALLINT nLength3, SQLCHAR *pszFKCatalogName, SQLSMALLINT nLength4, SQLCHAR *pszFKSchemaName, SQLSMALLINT nLength5, SQLCHAR *pszFKTableName, SQLSMALLINT nLength6 )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLForeignKeys( hStmt, pszPKCatalogName, nLength1, pszPKSchemaName, nLength2, pszPKTableName, nLength3, pszFKCatalogName, nLength4, pszFKSchemaName, nLength5, pszFKTableName, nLength6 );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_INVALID_HANDLE:
            case SQL_ERROR:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }

    } // while

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLProcedures
    Version Introduced: ODBC 1.0
    Standards Compliance: ODBC
    
    SQLProcedures returns the list of procedure names stored in a specific data source. Procedure is a generic term used to 
    describe an executable object, or a named entity that can be invoked using input and output parameters. For more 
    information on procedures, see the "Procedures" section in Chapter 9: Executing Statements.
*/
SQLRETURN MYODBCStatement::getProcedures( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszProcName, SQLSMALLINT nLength3 )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLProcedures( hStmt, pszCatalogName, nLength1, pszSchemaName, nLength2, pszProcName, nLength3 );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_INVALID_HANDLE:
            case SQL_ERROR:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }

    } // while

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLProcedureColumns
    Version Introduced: ODBC 1.0
    Standards Compliance: ODBC
    
    SQLProcedureColumns returns the list of input and output parameters, as well as the columns that 
    make up the result set for the specified procedures. The driver returns the information as a 
    result set on the specified statement.
*/
SQLRETURN MYODBCStatement::getProcedureColumns( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszProcName, SQLSMALLINT nLength3, SQLCHAR *pszColumnName, SQLSMALLINT nLength4 )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLProcedureColumns( hStmt, pszCatalogName, nLength1, pszSchemaName, nLength2, pszProcName, nLength3, pszColumnName, nLength4 );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLGetTypeInfo
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLGetTypeInfo returns information about data types supported by the data source. The driver returns 
    the information in the form of an SQL result set. The data types are intended for use in Data 
    Definition Language (DDL) statements.
    
    Important   Applications must use the type names returned in the TYPE_NAME column of the SQLGetTypeInfo 
                result set in ALTER TABLE and CREATE TABLE statements. SQLGetTypeInfo may return more than 
                one row with the same value in the DATA_TYPE column.
*/
SQLRETURN MYODBCStatement::getTypeInfo( SQLSMALLINT nDataType )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    time_t      timeStart;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    time( &timeStart );

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLGetTypeInfo( hStmt, nDataType );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_INVALID_HANDLE:
            case SQL_ERROR:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }

    } // while

    if ( !bCancelled )
    {
        SQLSMALLINT cols = -1;
        if(getNumResultCols(&cols) == SQL_SUCCESS)
            if(cols > 0)
                bHaveCursor = true;
    }

    nElapsedSeconds = difftime( time( NULL ), timeStart );

    return nReturn;
}

/*!
    SQLFetch
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    Summary
    
    SQLFetch fetches the next rowset of data from the result set and returns data for all 
    bound columns. 
*/
SQLRETURN MYODBCStatement::getFetch()
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    
    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLFetch( hStmt );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_NO_DATA:
                return nReturn;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    return nReturn;
}

/*!
    SQLGetData
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLGetData retrieves data for a single column in the result set. It can be called multiple times 
    to retrieve variable-length data in parts.
*/
SQLRETURN MYODBCStatement::getData( SQLUSMALLINT nColumnNumber, SQLSMALLINT nTargetType, SQLPOINTER pTargetValuePtr, SQLINTEGER nBufferLength, SQLINTEGER *pnStrLenOrIndPtr )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    
    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLGetData( hStmt, nColumnNumber, nTargetType, pTargetValuePtr, nBufferLength, pnStrLenOrIndPtr );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_NO_DATA:
                return nReturn;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    return nReturn;
}

/*!
    SQLColAttribute
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92
    
    SQLColAttribute returns descriptor information for a column in a result set. Descriptor information 
    is returned as a character string, a 32-bit descriptor-dependent value, or an integer value.
    
    Note   For more information about what the Driver Manager maps this function to when an ODBC 3.x 
           application is working with an ODBC 2.x driver, see "Mapping Replacement Functions for 
           Backward Compatibility of Applications" in Chapter 17: Programming Considerations.
*/
SQLRETURN MYODBCStatement::getColAttribute( SQLUSMALLINT nColumnNumber, SQLUSMALLINT nFieldIdentifier, SQLPOINTER nCharacterAttributePtr, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLengthPtr, SQLPOINTER pnNumericAttributePtr )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    
    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLColAttribute( hStmt, nColumnNumber, nFieldIdentifier, nCharacterAttributePtr, nBufferLength, pnStringLengthPtr, pnNumericAttributePtr );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    return nReturn;
}

/*!
    SQLNumResultCols
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLNumResultCols returns the number of columns in a result set.
*/
SQLRETURN MYODBCStatement::getNumResultCols( SQLSMALLINT *pnColumnCountPtr )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    
    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLNumResultCols( hStmt, pnColumnCountPtr );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    return nReturn;
}

/*!
    SQLRowCount
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLRowCount returns the number of rows affected by an UPDATE, INSERT, or DELETE statement; 
    an SQL_ADD, SQL_UPDATE_BY_BOOKMARK, or SQL_DELETE_BY_BOOKMARK operation in SQLBulkOperations; 
    or an SQL_UPDATE or SQL_DELETE operation in SQLSetPos.
*/
SQLRETURN MYODBCStatement::getRowCount( SQLINTEGER *pnRowCountPtr )
{
    SQLRETURN nReturn = SQLRowCount( hStmt, pnRowCountPtr );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            return SQL_ERROR;
    }

    return nReturn;
}

/*!
    getNewMessage
    
    All statement messages should be created with this. Doing so will ensure that QTMYODBCStatement
    will catch all messages and emit signal.
*/
MYODBCMessage *MYODBCStatement::getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState, char *pszMessage, SQLINTEGER nNativeCode )
{
    return new MYODBCMessage( nType, pszState, pszMessage, nNativeCode );
}

/*!
    Allocate the ODBC statement handle.

    ODBC++ will call this automatically as needed.
*/
SQLRETURN MYODBCStatement::doAlloc()
{
    // SANITY CHECKS
    if ( !pconnection )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): pconnection is NULL", -1 ) );
        return SQL_ERROR;
    }
    if ( pconnection->getDbc() == SQL_NULL_HDBC )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): hDbc not allocated", -1 ) );
        return SQL_ERROR;
    }
    if ( hStmt != SQL_NULL_HSTMT )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): hStmt already allocated", -1 ) );
        return SQL_ERROR;
    }
    if ( !pconnection->getIsConnected() )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): Not connected", -1 ) );
        return SQL_ERROR;
    }

    // DO IT
    SQLRETURN nReturn;
    nReturn = SQLAllocHandle( SQL_HANDLE_STMT, pconnection->getDbc(), &hStmt );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_ERROR:
            if ( hStmt != SQL_NULL_HSTMT )
            {
                if ( hStmt == pconnection->getDbc() )
                {
                    pconnection->doErrors( MYODBCMessage::MessageError );
                }
            }
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): SQL_ERROR", -1 ) );
            return nReturn;
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): SQL_INVALID_HANDLE", -1 ) );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): Unknown error", -1 ) );
            return SQL_ERROR;
    }

    // SET ATTRIBUTES HERE
    if ( bSQL_ATTR_ASYNC_ENABLE )
        setSQL_ATTR_ASYNC_ENABLE( nSQL_ATTR_ASYNC_ENABLE );

    // SET ATTRIBUTES HERE
    if ( bSQL_ATTR_CONCURRENCY )
        setSQL_ATTR_CONCURRENCY( nSQL_ATTR_CONCURRENCY );

// PAH - temp to get MS SQL Server to allow multiple active statements
// SQL_CONCUR_READ_ONLY, SQL_CONCUR_ROWVER, SQL_CONCUR_VALUES, or SQL_CONCUR_LOCK.
    SQLSetStmtAttr( hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, SQL_IS_INTEGER );
    SQLSetStmtAttr( hStmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_INTEGER );

    return nReturn;
}

/*!
    Free the ODBC handle.

    ODBC++ will call this automatically as needed.
*/
SQLRETURN MYODBCStatement::doFree( bool bForce )
{
    // SANITY CHECKS
    if ( hStmt == SQL_NULL_HSTMT )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doFree(): hStmt not allocated", -1 ) );
        return SQL_ERROR;
    }

    // DO IT
    SQLRETURN nReturn;
    nReturn = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        doErrors( MYODBCMessage::MessageError );
        if ( !bForce )
            return nReturn;
    }

    hStmt       = SQL_NULL_HSTMT;
    bHaveCursor = false;

    return nReturn;
}

/*!
    SQLCloseCursor
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92
    
    SQLCloseCursor closes a cursor that has been opened on a statement and discards pending results.
    
    ODBC++ will call this automatically as needed.
*/
SQLRETURN MYODBCStatement::doCloseCursor()
{
    // DO IT
    SQLRETURN nReturn;

    nReturn = SQLCloseCursor( hStmt );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            return SQL_ERROR;
    }

    bHaveCursor = false;

    return nReturn;
}

/*!
    SQLPrepare
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLPrepare prepares an SQL string for execution.

    This will pre-processes the SQL statement ensuring that it is valid and then
    using it in subsequent calls to doExecute(). 
    
    Call this with your SQL statement then make 1 or more calls to
    doExecute().
    
    Consider using doExecuteDirect() if you only intend to make one doExecute() 
    call with the given SQL statement.
*/
SQLRETURN MYODBCStatement::doPrepare( SQLCHAR *pszStatementText, SQLINTEGER nTextLength )
{
    bool        bCancelled  = false;
    SQLRETURN   nReturn;
    
    // cleanup
    if ( hStmt == SQL_NULL_HSTMT )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doAlloc()) ) )
            return nReturn; // message from doAlloc()
    }

    if ( bHaveCursor )
    {
        if ( !SQL_SUCCEEDED( (nReturn = doCloseCursor()) ) )
            return nReturn; // message from doCloseCursor()
    }

    // do it
    nReturn = SQL_STILL_EXECUTING;
    while ( nReturn == SQL_STILL_EXECUTING )
    {
        nReturn = SQLPrepare( hStmt, pszStatementText, nTextLength );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                break;
            case SQL_SUCCESS_WITH_INFO:
                doErrors( MYODBCMessage::MessageInfo );
                break;
            case SQL_STILL_EXECUTING:
                if ( !bCancelled && !doWaiting() )
                {
                    if ( SQL_SUCCEEDED( doCancel() ) )
                        bCancelled = true;  // Cancelled! We still need to loop to give the driver time to cleanup.
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
                doErrors( MYODBCMessage::MessageError );
                return nReturn;
            default:
                doErrors( MYODBCMessage::MessageError );
                return SQL_ERROR;
        }
    }

    return nReturn;
}

/*!
    SQLBindCol
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLBindCol binds application data buffers to columns in the result set.
*/
SQLRETURN MYODBCStatement::doBindCol( SQLUSMALLINT nColumnNumber, SQLSMALLINT nTargetType, SQLPOINTER pTargetValuePtr, SQLLEN nBufferLength, SQLLEN *pnStrLen_or_Ind )
{
    SQLRETURN nReturn;
    
    nReturn = SQLBindCol( hStmt, nColumnNumber, nTargetType, pTargetValuePtr, nBufferLength, pnStrLen_or_Ind );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            return SQL_ERROR;
    }

    return nReturn;
}

/*!
    SQLCancel
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLCancel cancels the processing on a statement.
*/
SQLRETURN MYODBCStatement::doCancel()
{
    SQLRETURN nReturn = SQLCancel( hStmt );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            return SQL_ERROR;
    }

    return nReturn;
}

/*!
    Do something between asynch calls.
    
    This gets called each iteration through a pooling loop when the
    ODBC asynch option is turned on and the driver supports asynch calls.
    
    So one could derive a new class from here and replace doWaiting()
    with something more useful.
    
    Whatever is done in here should not take more than a moment to process
    because we want to check out the progress of our asynch call fairly
    frequently... perhaps every second or so.
    
    Return false to cancel asynch operation.
*/
bool MYODBCStatement::doWaiting()
{
    printf( "[PAH][%s][%d] Looks like asynch option supported by this driver.\n", __FILE__, __LINE__ );
#ifndef Q_WS_WIN
    sleep( 1 ); // just slow the polling down a bit
#endif
    return true;
}

/*!
    doErrors 
    
    Capture ODBC messages.
*/
void MYODBCStatement::doErrors( MYODBCMessage::MessageTypes nType )
{
    SQLRETURN       nReturn = SQL_SUCCESS;
    SQLCHAR		    szMessage[501];
    SQLCHAR	        szState[10];
    SQLINTEGER      nNativeCode;
    SQLSMALLINT     nReturnLength;
    MYODBCMessage *   pmessage = 0;

    SQLSMALLINT     nRecord = 1;

    while ( SQL_SUCCEEDED( nReturn ) )
    {
        *szState    = '\0';
        *szMessage  = '\0';
        nReturn = SQLGetDiagRec( SQL_HANDLE_STMT, hStmt, nRecord, szState, &nNativeCode, szMessage, sizeof(szMessage) - 1, &nReturnLength );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                pmessage = getNewMessage( nType, (char*)szState, (char*)szMessage, nNativeCode );
                break;
            case SQL_SUCCESS_WITH_INFO:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source. Buffer to small.", -1 );
                break;
            case SQL_INVALID_HANDLE:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source. Invalid handle.", -1 );
                break;
            case SQL_ERROR:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source.", -1 );
                break;
            case SQL_NO_DATA:
                // pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source. No data.", -1 );
                // break;
                return;
            default:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source. Unknown return code from SQLGetDiagRec().", -1 );
        }

        objectlistMessages.doAppend( pmessage );
        nRecord++;
    }
}


