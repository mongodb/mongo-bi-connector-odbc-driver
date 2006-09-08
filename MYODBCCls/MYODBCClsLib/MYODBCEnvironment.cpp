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

#include <MYODBCEnvironment.h>

MYODBCEnvironment::MYODBCEnvironment()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    hEnv            = SQL_NULL_HENV;
    objectlistMessages.setAutoDelete( true );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYODBCEnvironment::~MYODBCEnvironment()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    // clear messages
    // - auto delete should be On... so objects will be deleted
    objectlistMessages.doClear();

    if ( hEnv != SQL_NULL_HENV )
    {
        doFree();
    }

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    SQLDrivers
    Version Introduced: ODBC 2.0
    Standards Compliance: ODBC
    
    SQLDrivers lists driver descriptions and driver attribute keywords. This 
    function is implemented solely by the Driver Manager.
*/
SQLRETURN MYODBCEnvironment::getDriver( SQLUSMALLINT nDirection, SQLCHAR *pszDriverDescription, SQLSMALLINT nBufferLength1, SQLSMALLINT *pnDescriptionLengthPtr, SQLCHAR *pszDriverAttributes, SQLSMALLINT nBufferLength2, SQLSMALLINT *pnAttributesLengthPtr )
{
    // ensure we have an hEnv
    if ( hEnv == SQL_NULL_HENV )
    {
        if ( !doAlloc() )
            return SQL_ERROR;
    }

    //
    SQLRETURN nReturn = SQLDrivers( hEnv, nDirection, pszDriverDescription, nBufferLength1, pnDescriptionLengthPtr, pszDriverAttributes, nBufferLength2, pnAttributesLengthPtr );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
        case SQL_NO_DATA:
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
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getDriver(): Invalid return value from SQLDrivers()", -1 ) );
            return SQL_ERROR;
    }

    return nReturn;
}

/*!
    SQLDataSource
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92

    SQLDataSources returns information about a data source. This function is implemented solely 
    by the Driver Manager.
*/
SQLRETURN MYODBCEnvironment::getDataSource( SQLUSMALLINT nDirection, SQLCHAR *pszServerName, SQLSMALLINT nBufferLength1, SQLSMALLINT *pnNameLength1Ptr, SQLCHAR *pszDescription, SQLSMALLINT nBufferLength2, SQLSMALLINT *pnNameLength2Ptr )
{
    // ensure we have an hEnv
    if ( hEnv == SQL_NULL_HENV )
    {
        if ( !doAlloc() )
            return SQL_ERROR;
    }

    //
    SQLRETURN nReturn = SQLDataSources( hEnv, nDirection, pszServerName, nBufferLength1, pnNameLength1Ptr, pszDescription, nBufferLength2, pnNameLength2Ptr );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
        case SQL_NO_DATA:
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
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getDataSource(): Invalid return value from SQLDataSources()", -1 ) );
            return SQL_ERROR;
    }

    return nReturn;
}

/*!
    getNewMessage
    
    All environment messages should be created with this. Doing so will ensure that QTMYODBCEnvironment
    will catch all messages and emit signal.
*/
MYODBCMessage *MYODBCEnvironment::getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState, char *pszMessage, SQLINTEGER nNativeCode )
{
    return new MYODBCMessage( nType, pszState, pszMessage, nNativeCode );
}

/*!
    SQLManageDataSources 
    Version Introduced: ODBC 2.0
    Supported by: Installer library
    
    SQLManageDataSources displays a dialog box with which users can set up, add, and delete 
    data sources in the system information.
*/
BOOL MYODBCEnvironment::doManageDataSources( HWND hWnd )
{
    BOOL bReturn = SQLManageDataSources( hWnd );

    if ( !bReturn )
    {
        /* TODO:
         *
         * Call SQLInstallerError() here to get error code
         * and then generate an MYODBCMessage.
         *
         */
    }

    return bReturn;
}

/*!
    SQLAllocHandle
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92    
    
    SQLAllocHandle allocates an environment, connection, statement, or descriptor handle.
    Note   This function is a generic function for allocating handles that replaces the ODBC 2.0 
           functions SQLAllocConnect, SQLAllocEnv, and SQLAllocStmt. To allow applications calling 
           SQLAllocHandle to work with ODBC 2.x drivers, a call to SQLAllocHandle is mapped in the 
           Driver Manager to SQLAllocConnect, SQLAllocEnv, or SQLAllocStmt, as appropriate. For 
           more information, see "Comments." For more information about what the Driver Manager 
           maps this function to when an ODBC 3.x application is working with an ODBC 2.x driver, 
           see "Mapping Replacement Functions for Backward Compatibility of Applications" in 
           Chapter 17: Programming Considerations.    
*/
BOOL MYODBCEnvironment::doAlloc()
{
    // SANITY CHECKS
    if ( hEnv != SQL_NULL_HENV )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): hEnv already allocated", -1 ) );
        return false;
    }

    // DO IT
    SQLRETURN nReturn;
    nReturn = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv );
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
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): Failed to allocate environment", -1 ) );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): Invalid return value from SQLAllocHandle()", -1 ) );
            return SQL_ERROR;
    }

    // SET ATTRIBUTES
    nReturn = SQLSetEnvAttr( hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0 );
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
    }

    return true;
}

/*!
    SQLFreeHandle
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92
    
    SQLFreeHandle frees resources associated with a specific environment, connection, statement, or descriptor handle.
    Note   This function is a generic function for freeing handles. It replaces the ODBC 2.0 functions 
           SQLFreeConnect (for freeing a connection handle) and SQLFreeEnv (for freeing an environment handle). 
           SQLFreeConnect and SQLFreeEnv are both deprecated in ODBC 3.x. SQLFreeHandle also replaces the ODBC 2.0 
           function SQLFreeStmt (with the SQL_DROP Option) for freeing a statement handle. For more information, see 
           "Comments." For more information about what the Driver Manager maps this function to when an ODBC 3.x 
           application is working with an ODBC 2.x driver, see "Mapping Replacement Functions for Backward 
           Compatibility of Applications" in Chapter 17: Programming Considerations.
*/
BOOL MYODBCEnvironment::doFree()
{
    // SANITY CHECKS
    if ( hEnv == SQL_NULL_HENV )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doFree(): hEnv not allocated", -1 ) );
        return false;
    }

    // Call doFree() on child MYODBCConnection's first
    MYODBCObject *        pobject = objectlistZChildren.getFirst();
    MYODBCConnection *    pconnection;

    while ( pobject )
    {
//        if ( strcmp( pobject->getZClassName(), "MYODBCConnection" ) == 0 )
        {
            pconnection = (MYODBCConnection*)pobject;
            if ( pconnection->getDbc() != SQL_NULL_HDBC )
			{
				if ( !SQL_SUCCEEDED( pconnection->doFree() ) )
					return false;
			}
        }
        pobject = pobject->getZNext();
    }

    // DO IT
    SQLRETURN nReturn;
    nReturn = SQLFreeHandle( SQL_HANDLE_ENV, hEnv );
    hEnv = SQL_NULL_HENV;

    return true;
}

/*!
    Capture ODBC messages.
*/
void MYODBCEnvironment::doErrors( MYODBCMessage::MessageTypes nType )
{
    SQLRETURN       nReturn = SQL_SUCCESS;
    SQLCHAR		    szMessage[501];
    SQLCHAR	        szState[10];
    SQLINTEGER      nNativeCode;
    SQLSMALLINT     nReturnLength;
    MYODBCMessage *   pmessage = 0;

    SQLSMALLINT nRecord         = 1;

    while ( SQL_SUCCEEDED( nReturn ) )
    {
        *szState    = '\0';
        *szMessage  = '\0';
        nReturn = SQLGetDiagRec( SQL_HANDLE_ENV, hEnv, nRecord, szState, &nNativeCode, szMessage, sizeof(szMessage) - 1, &nReturnLength );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
                pmessage = getNewMessage( nType, (char*)szState, (char*)szMessage, nNativeCode );
                break;
            case SQL_SUCCESS_WITH_INFO:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[QTODBC++] doErrors(): Failed to get error information from driver or data source. Buffer to small.", -1 );
                break;
            case SQL_INVALID_HANDLE:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[QTODBC++] doErrors(): Failed to get error information from driver or data source. Invalid handle.", -1 );
                break;
            case SQL_ERROR:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[QTODBC++] doErrors(): Failed to get error information from driver or data source.", -1 );
                break;
            case SQL_NO_DATA:
                // pmessage = new MYODBCMessage( MYODBCMessage::MessageError, 0, "[QTODBC++] doErrors(): Failed to get error information from driver or data source. No data.", -1 );
                // break;
                return;
            default:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[QTODBC++] doErrors(): Failed to get error information from driver or data source. Unknown return code from SQLGetDiagRec().", -1 );
        }

        objectlistMessages.doAppend( pmessage );
        nRecord++;
    }
}

