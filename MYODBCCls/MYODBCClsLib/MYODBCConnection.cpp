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

// to enable strndup
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <MYODBCConnection.h>
#include <MYODBCEnvironment.h>

MYODBCConnection::MYODBCConnection( MYODBCEnvironment *penvironment )
    : MYODBCObject( penvironment )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    this->penvironment  = penvironment;
    hDbc                = SQL_NULL_HDBC;
    bConnected          = false;
    objectlistMessages.setAutoDelete( true );

    // Flag - do not set any attributes values unless flag is set
    bSQL_ATTR_ACCESS_MODE           = false;
    bSQL_ATTR_ASYNC_ENABLE          = false; 
    bSQL_ATTR_CONNECTION_TIMEOUT    = false;
    bSQL_ATTR_CURRENT_CATALOG       = false;
    bSQL_ATTR_LOGIN_TIMEOUT         = false;
    bSQL_ATTR_PACKET_SIZE           = false;
    bSQL_ATTR_TRACE                 = false;
    bSQL_ATTR_TRACEFILE             = false;

    // Value - default attributes according to ODBC specification
    nSQL_ATTR_ACCESS_MODE           = SQL_MODE_READ_WRITE;  //
    nSQL_ATTR_ASYNC_ENABLE          = SQL_ASYNC_ENABLE_OFF; //
    nSQL_ATTR_CONNECTION_TIMEOUT    = 0;                    // no standard default but 0 will turn off timeout
    pszSQL_ATTR_CURRENT_CATALOG     = NULL;                 // no standard default
    nSQL_ATTR_LOGIN_TIMEOUT         = 0;                    // no standard default but 0 will turn off timeout
    nSQL_ATTR_PACKET_SIZE           = 1024;                 // no standard default
    nSQL_ATTR_TRACE                 = SQL_OPT_TRACE_OFF;    //
    pszSQL_ATTR_TRACEFILE           = NULL;                 // this should actually be read from ODBC sysinfo

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYODBCConnection::~MYODBCConnection()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    // clear messages
    // - auto delete should be On... so objects will be deleted
    objectlistMessages.doClear();

    if ( hDbc != SQL_NULL_HDBC )
        doFree( true );

    if ( pszSQL_ATTR_CURRENT_CATALOG )
        free( pszSQL_ATTR_CURRENT_CATALOG );
    if ( pszSQL_ATTR_TRACEFILE )
        free( pszSQL_ATTR_TRACEFILE );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    SQL_ATTR_ACCESS_MODE
    ODBC 1.0
    
    An SQLUINTEGER value. SQL_MODE_READ_ONLY is used by the driver or data source as 
    an indicator that the connection is not required to support SQL statements that 
    cause updates to occur. This mode can be used to optimize locking strategies, 
    transaction management, or other areas as appropriate to the driver or data source. 
    The driver is not required to prevent such statements from being submitted to the 
    data source. The behavior of the driver and data source when asked to process SQL 
    statements that are not read-only during a read-only connection is 
    implementation-defined. SQL_MODE_READ_WRITE is the default.
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_ACCESS_MODE( SQLUINTEGER nSQL_ATTR_ACCESS_MODE )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks
    switch ( nSQL_ATTR_ACCESS_MODE )
    {
        case SQL_MODE_READ_ONLY:
        case SQL_MODE_READ_WRITE:
            break;
        default:
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setSQL_ATTR_ACCESS_MODE(): Invalid argument", -1 ) );
            return SQL_ERROR;
    }

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_ACCESS_MODE, (SQLPOINTER)nSQL_ATTR_ACCESS_MODE, SQL_IS_UINTEGER );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_ACCESS_MODE       = true;
    this->nSQL_ATTR_ACCESS_MODE = nSQL_ATTR_ACCESS_MODE;

    return nReturn;
}

/*!
    SQL_ATTR_ASYNC_ENABLE
    ODBC 3.0

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
SQLRETURN MYODBCConnection::setSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER nSQL_ATTR_ASYNC_ENABLE )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks
    switch ( nSQL_ATTR_ASYNC_ENABLE )
    {
        case SQL_ASYNC_ENABLE_OFF:
        case SQL_ASYNC_ENABLE_ON:
            break;
        default:
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setSQL_ATTR_ASYNC_ENABLE(): Invalid argument", -1 ) );
            return SQL_ERROR;
    }

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)nSQL_ATTR_ASYNC_ENABLE, SQL_IS_UINTEGER );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_ASYNC_ENABLE          = true;
    this->nSQL_ATTR_ASYNC_ENABLE    = nSQL_ATTR_ASYNC_ENABLE;

    return nReturn;
}

/*!
    SQL_ATTR_CONNECTION_TIMEOUT
    ODBC 3.0

    An SQLUINTEGER value corresponding to the number of seconds to wait for any request 
    on the connection to complete before returning to the application. The driver should 
    return SQLSTATE HYT00 (Timeout expired) anytime that it is possible to time out in a 
    situation not associated with query execution or login.
    
    If ValuePtr is equal to 0 (the default), there is no timeout.
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_CONNECTION_TIMEOUT( SQLUINTEGER nSQL_ATTR_CONNECTION_TIMEOUT )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)nSQL_ATTR_CONNECTION_TIMEOUT, SQL_IS_UINTEGER );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_CONNECTION_TIMEOUT        = true;
    this->nSQL_ATTR_CONNECTION_TIMEOUT  = nSQL_ATTR_CONNECTION_TIMEOUT;

    return nReturn;
}

/*!
    SQL_ATTR_CURRENT_CATALOG
    ODBC 2.0

    A character string containing the name of the catalog to be used by the data source. For 
    example, in SQL Server, the catalog is a database, so the driver sends a USE database 
    statement to the data source, where database is the database specified in *ValuePtr. For 
    a single-tier driver, the catalog might be a directory, so the driver changes its 
    current directory to the directory specified in *ValuePtr.
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_CURRENT_CATALOG( SQLCHAR *pszSQL_ATTR_CURRENT_CATALOG )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)pszSQL_ATTR_CURRENT_CATALOG, SQL_NTS );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_CURRENT_CATALOG          = true;
    if ( this->pszSQL_ATTR_CURRENT_CATALOG )
        free( this->pszSQL_ATTR_CURRENT_CATALOG );
    this->pszSQL_ATTR_CURRENT_CATALOG = NULL;
    if ( pszSQL_ATTR_CURRENT_CATALOG )
        this->pszSQL_ATTR_CURRENT_CATALOG = (SQLCHAR*)strdup( (const char*)pszSQL_ATTR_CURRENT_CATALOG );

    return nReturn;
}

/*!
    SQL_ATTR_LOGIN_TIMEOUT
    ODBC 1.0

    An SQLUINTEGER value corresponding to the number of seconds to wait for a login request 
    to complete before returning to the application. The default is driver-dependent. If 
    ValuePtr is 0, the timeout is disabled and a connection attempt will wait indefinitely.
    
    If the specified timeout exceeds the maximum login timeout in the data source, the driver 
    substitutes that value and returns SQLSTATE 01S02 (Option value changed).
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_LOGIN_TIMEOUT( SQLUINTEGER nSQL_ATTR_LOGIN_TIMEOUT )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)nSQL_ATTR_LOGIN_TIMEOUT, SQL_IS_UINTEGER );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_LOGIN_TIMEOUT          = true;
    this->nSQL_ATTR_LOGIN_TIMEOUT    = nSQL_ATTR_LOGIN_TIMEOUT;

    return nReturn;
}

/*!
    SQL_ATTR_PACKET_SIZE
    ODBC 2.0

    An SQLUINTEGER value specifying the network packet size in bytes.
    Note   Many data sources either do not support this option or only 
           can return but not set the network packet size.
    
    If the specified size exceeds the maximum packet size or is smaller 
    than the minimum packet size, the driver substitutes that value and 
    returns SQLSTATE 01S02 (Option value changed).
    
    If the application sets packet size after a connection has already been 
    made, the driver will return SQLSTATE HY011 (Attribute cannot be set now).
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_PACKET_SIZE( SQLUINTEGER nSQL_ATTR_PACKET_SIZE )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_PACKET_SIZE, (SQLPOINTER)nSQL_ATTR_PACKET_SIZE, SQL_IS_UINTEGER );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_PACKET_SIZE          = true;
    this->nSQL_ATTR_PACKET_SIZE    = nSQL_ATTR_PACKET_SIZE;

    return nReturn;
}

/*!
    SQL_ATTR_TRACE
    ODBC 1.0

    An SQLUINTEGER value telling the Driver Manager whether to perform tracing:
    
    SQL_OPT_TRACE_OFF = Tracing off (the default)
    
    SQL_OPT_TRACE_ON = Tracing on
    
    When tracing is on, the Driver Manager writes each ODBC function call to the trace file.
    Note   When tracing is on, the Driver Manager can return SQLSTATE IM013 (Trace file 
           error) from any function.
    
    An application specifies a trace file with the SQL_ATTR_TRACEFILE option. If the file 
    already exists, the Driver Manager appends to the file. Otherwise, it creates the file. 
    If tracing is on and no trace file has been specified, the Driver Manager writes to the 
    file SQL.LOG in the root directory.
    
    An application can set the variable ODBCSharedTraceFlag to enable tracing dynamically. 
    Tracing is then enabled for all ODBC applications currently running. If an application 
    turns tracing off, it is turned off only for that application.
    
    If the Trace keyword in the system information is set to 1 when an application calls 
    SQLAllocHandle with a HandleType of SQL_HANDLE_ENV, tracing is enabled for all handles. 
    It is enabled only for the application that called SQLAllocHandle.
    
    Calling SQLSetConnectAttr with an Attribute of SQL_ATTR_TRACE does not require that the 
    ConnectionHandle argument be valid and will not return SQL_ERROR if ConnectionHandle is 
    NULL. This attribute applies to all connections.
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_TRACE( SQLUINTEGER nSQL_ATTR_TRACE )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks
    switch ( nSQL_ATTR_ASYNC_ENABLE )
    {
        case SQL_OPT_TRACE_OFF:
        case SQL_OPT_TRACE_ON:
            break;
        default:
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setSQL_ATTR_ASYNC_ENABLE(): Invalid argument", -1 ) );
            return SQL_ERROR;
    }

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_TRACE, (SQLPOINTER)nSQL_ATTR_TRACE, SQL_IS_UINTEGER );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_TRACE          = true;
    this->nSQL_ATTR_TRACE    = nSQL_ATTR_TRACE;

    return nReturn;
}

/*!
    SQL_ATTR_TRACEFILE
    ODBC 1.0
    
    A null-terminated character string containing the name of the trace file.
    
    The default value of the SQL_ATTR_TRACEFILE attribute is specified with the 
    TraceFile keyword in the system information. For more information, see 
    "ODBC Subkey" in Chapter 19: Configuring Data Sources.
    
    Calling SQLSetConnectAttr with an Attribute of SQL_ATTR_ TRACEFILE does not 
    require the ConnectionHandle argument to be valid and will not return SQL_ERROR 
    if ConnectionHandle is invalid. This attribute applies to all connections.
*/
SQLRETURN MYODBCConnection::setSQL_ATTR_TRACEFILE( SQLCHAR *pszSQL_ATTR_TRACEFILE )
{
    SQLRETURN nReturn = SQL_SUCCESS;

    // sanity checks

    // apply now
    if ( hDbc != SQL_NULL_HDBC )
    {
        nReturn = setConnectAttr( SQL_ATTR_TRACEFILE, (SQLPOINTER)pszSQL_ATTR_TRACEFILE, SQL_NTS );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;
    }

    // accept
    bSQL_ATTR_TRACEFILE          = true;
    if ( this->pszSQL_ATTR_TRACEFILE )
        free( this->pszSQL_ATTR_TRACEFILE );
    this->pszSQL_ATTR_TRACEFILE = NULL;
    if ( pszSQL_ATTR_TRACEFILE )
        this->pszSQL_ATTR_TRACEFILE = (SQLCHAR*)strdup( (const char*)pszSQL_ATTR_TRACEFILE );

    return nReturn;
}

/*!
    Returns SQL_ATTR_ACCESS_MODE. 
    
    See setSQL_ATTR_ACCESS_MODE() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_ACCESS_MODE( SQLUINTEGER *pnSQL_ATTR_ACCESS_MODE )
{
    // sanity checks
    if ( !pnSQL_ATTR_ACCESS_MODE )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_ACCESS_MODE(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        *pnSQL_ATTR_ACCESS_MODE = nSQL_ATTR_ASYNC_ENABLE;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_ACCESS_MODE(): Using cached value", -1 ) );
        return SQL_SUCCESS_WITH_INFO;
    }

    // get actual val
    SQLINTEGER nRetSize;
    SQLRETURN nReturn = getConnectAttr( SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)pnSQL_ATTR_ACCESS_MODE, SQL_IS_UINTEGER, &nRetSize  );

    if ( SQL_SUCCEEDED( nReturn ) )
        nSQL_ATTR_ACCESS_MODE = *pnSQL_ATTR_ACCESS_MODE;

    return nReturn;
}

/*!
    Returns SQL_ATTR_ASYNC_ENABLE. 
    
    See setSQL_ATTR_ASYNC_ENABLE() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER *pnSQL_ATTR_ASYNC_ENABLE )
{
    // sanity checks
    if ( !pnSQL_ATTR_ASYNC_ENABLE )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_ASYNC_ENABLE(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        *pnSQL_ATTR_ASYNC_ENABLE = nSQL_ATTR_ASYNC_ENABLE;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_ASYNC_ENABLE(): Using cached value", -1 ) );
        return SQL_SUCCESS_WITH_INFO;
    }

    // get actual val
    SQLINTEGER nRetSize;
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)pnSQL_ATTR_ASYNC_ENABLE, SQL_IS_UINTEGER, &nRetSize  );

    if ( SQL_SUCCEEDED( nReturn ) )
        nSQL_ATTR_ASYNC_ENABLE = *pnSQL_ATTR_ASYNC_ENABLE;

    return nReturn;
}

/*!
    Returns SQL_ATTR_CONNECTION_TIMEOUT. 
    
    See setSQL_ATTR_CONNECTION_TIMEOUT() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_CONNECTION_TIMEOUT( SQLUINTEGER *pnSQL_ATTR_CONNECTION_TIMEOUT )
{
    // sanity checks
    if ( !pnSQL_ATTR_CONNECTION_TIMEOUT )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_CONNECTION_TIMEOUT(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        *pnSQL_ATTR_CONNECTION_TIMEOUT = nSQL_ATTR_CONNECTION_TIMEOUT;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_CONNECTION_TIMEOUT(): Using cached value", -1 ) );
        return SQL_SUCCESS_WITH_INFO;
    }

    // get actual val
    SQLINTEGER nRetSize;
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)pnSQL_ATTR_CONNECTION_TIMEOUT, SQL_IS_UINTEGER, &nRetSize  );

    if ( SQL_SUCCEEDED( nReturn ) )
        nSQL_ATTR_CONNECTION_TIMEOUT = *pnSQL_ATTR_CONNECTION_TIMEOUT;

    return nReturn;
}

/*!
    Returns SQL_ATTR_ASYNC_ENABLE. 
    
    See setSQL_ATTR_ASYNC_ENABLE() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_CURRENT_CATALOG( SQLCHAR *pszSQL_ATTR_CURRENT_CATALOG, SQLINTEGER nLength, SQLINTEGER *pnReturnLength )
{
    // sanity checks
    if ( !pszSQL_ATTR_CURRENT_CATALOG )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_CURRENT_CATALOG(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }
    if ( nLength < 1 )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_CURRENT_CATALOG(): Invalid nLength", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !pszSQL_ATTR_CURRENT_CATALOG )
            return SQL_NO_DATA;

        strncpy( (char *)pszSQL_ATTR_CURRENT_CATALOG, (const char*)this->pszSQL_ATTR_CURRENT_CATALOG, nLength );
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_CURRENT_CATALOG(): Using cached value", -1 ) );

        return SQL_SUCCESS_WITH_INFO;
    }

    //
    if ( !this->pszSQL_ATTR_CURRENT_CATALOG )
        free( this->pszSQL_ATTR_CURRENT_CATALOG );
    this->pszSQL_ATTR_CURRENT_CATALOG = NULL;

    // get actual val
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)pszSQL_ATTR_CURRENT_CATALOG, nLength, pnReturnLength );

    if ( SQL_SUCCEEDED( nReturn ) )
    {
        this->pszSQL_ATTR_CURRENT_CATALOG = (SQLCHAR *)strdup( (const char*)pszSQL_ATTR_CURRENT_CATALOG );
    }

    return nReturn;
}

/*!
    Returns SQL_ATTR_LOGIN_TIMEOUT. 
    
    See setSQL_ATTR_LOGIN_TIMEOUT() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_LOGIN_TIMEOUT( SQLUINTEGER *pnSQL_ATTR_LOGIN_TIMEOUT )
{
    // sanity checks
    if ( !pnSQL_ATTR_LOGIN_TIMEOUT )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_LOGIN_TIMEOUT(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        *pnSQL_ATTR_LOGIN_TIMEOUT = nSQL_ATTR_LOGIN_TIMEOUT;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_LOGIN_TIMEOUT(): Using cached value", -1 ) );
        return SQL_SUCCESS_WITH_INFO;
    }

    // get actual val
    SQLINTEGER nRetSize;
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)pnSQL_ATTR_LOGIN_TIMEOUT, SQL_IS_UINTEGER, &nRetSize  );

    if ( SQL_SUCCEEDED( nReturn ) )
        nSQL_ATTR_LOGIN_TIMEOUT = *pnSQL_ATTR_LOGIN_TIMEOUT;

    return nReturn;
}

/*!
    Returns SQL_ATTR_PACKET_SIZE. 
    
    See setSQL_ATTR_PACKET_SIZE() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_PACKET_SIZE( SQLUINTEGER *pnSQL_ATTR_PACKET_SIZE )
{
    // sanity checks
    if ( !pnSQL_ATTR_PACKET_SIZE )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_PACKET_SIZE(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        *pnSQL_ATTR_PACKET_SIZE = nSQL_ATTR_PACKET_SIZE;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_PACKET_SIZE(): Using cached value", -1 ) );
        return SQL_SUCCESS_WITH_INFO;
    }

    // get actual val
    SQLINTEGER nRetSize;
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_PACKET_SIZE, (SQLPOINTER)pnSQL_ATTR_PACKET_SIZE, SQL_IS_UINTEGER, &nRetSize  );

    if ( SQL_SUCCEEDED( nReturn ) )
        nSQL_ATTR_PACKET_SIZE = *pnSQL_ATTR_PACKET_SIZE;

    return nReturn;
}

/*!
    Returns SQL_ATTR_TRACE. 
    
    See setSQL_ATTR_TRACE() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_TRACE( SQLUINTEGER *pnSQL_ATTR_TRACE )
{
    // sanity checks
    if ( !pnSQL_ATTR_TRACE )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_TRACE(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        *pnSQL_ATTR_TRACE = nSQL_ATTR_TRACE;
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_TRACE(): Using cached value", -1 ) );
        return SQL_SUCCESS_WITH_INFO;
    }

    // get actual val
    SQLINTEGER nRetSize;
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_TRACE, (SQLPOINTER)pnSQL_ATTR_TRACE, SQL_IS_UINTEGER, &nRetSize  );

    if ( SQL_SUCCEEDED( nReturn ) )
        nSQL_ATTR_TRACE = *pnSQL_ATTR_TRACE;

    return nReturn;
}

/*!
    Returns SQL_ATTR_TRACEFILE. 
    
    See setSQL_ATTR_TRACEFILE() for details.
*/
SQLRETURN MYODBCConnection::getSQL_ATTR_TRACEFILE( SQLCHAR *pszSQL_ATTR_TRACEFILE, SQLINTEGER nLength, SQLINTEGER *pnReturnLength )
{
    // sanity checks
    if ( !pszSQL_ATTR_TRACEFILE )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_TRACEFILE(): Invalid pointer", -1 ) );
        return SQL_ERROR;
    }
    if ( nLength < 1 )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getSQL_ATTR_TRACEFILE(): Invalid nLength", -1 ) );
        return SQL_ERROR;
    }

    // use cached val
    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !pszSQL_ATTR_TRACEFILE )
            return SQL_NO_DATA;

        strncpy( (char *)pszSQL_ATTR_TRACEFILE, (const char*)this->pszSQL_ATTR_TRACEFILE, nLength );
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageInfo, 0, "[ODBC++] getSQL_ATTR_TRACEFILE(): Using cached value", -1 ) );

        return SQL_SUCCESS_WITH_INFO;
    }

    //
    if ( !this->pszSQL_ATTR_TRACEFILE )
        free( this->pszSQL_ATTR_TRACEFILE );
    this->pszSQL_ATTR_TRACEFILE = NULL;

    // get actual val
    SQLRETURN nReturn = getConnectAttr(  SQL_ATTR_TRACEFILE, (SQLPOINTER)pszSQL_ATTR_TRACEFILE, nLength, pnReturnLength );

    if ( SQL_SUCCEEDED( nReturn ) )
    {
        this->pszSQL_ATTR_TRACEFILE = (SQLCHAR*)strdup( (const char *)pszSQL_ATTR_TRACEFILE );
    }

    return nReturn;
}

/*!
    An SQLUINTEGER value indicating the level of asynchronous support in the driver:
    
    SQL_AM_CONNECTION = Connection level asynchronous execution is supported. Either 
    all statement handles associated with a given connection handle are in asynchronous 
    mode or all are in synchronous mode. A statement handle on a connection cannot be 
    in asynchronous mode while another statement handle on the same connection is in 
    synchronous mode, and vice versa.
    
    SQL_AM_STATEMENT = Statement level asynchronous execution is supported. Some 
    statement handles associated with a connection handle can be in asynchronous mode, 
    while other statement handles on the same connection are in synchronous mode.
    
    SQL_AM_NONE = Asynchronous mode is not supported.
*/
/*
SQLRETURN MYODBCConnection::getSQL_ASYNC_MODE( SQLUINTEGER *pn )
{
    // DO IT
    SQLSMALLINT nRetSize;

    SQLRETURN nReturn = SQLGetInfo( hDbc, SQL_ASYNC_MODE, (SQLPOINTER)pn, SQL_IS_UINTEGER, &nRetSize  );
    if ( nReturn == SQL_SUCCESS )
    {
printf( "[PAH][%s][%d]\n", __FILE__, __LINE__ );
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
printf( "[PAH][%s][%d]\n", __FILE__, __LINE__ );
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
printf( "[PAH][%s][%d] %p %p = %ld %d\n", __FILE__, __LINE__, hDbc, pn, *pn, nReturn );
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] SQLGetInfo(): Failed to get SQL_ASYNC_MODE", -1 ) );
        doErrors( MYODBCMessage::MessageError );
        return nReturn;
    }

    return nReturn;
}
*/

/*!
    getIsConnected
    
    Returns true if we think there is a connection else false.
*/
bool MYODBCConnection::getIsConnected()
{
/*
    // SANITY CHECKS
    if ( hDbc == SQL_NULL_HDBC )
        return false;

    // DO IT
    SQLRETURN   nReturn;
    SQLUINTEGER nValue;
    SQLINTEGER  nRetSize;

    nReturn = SQLGetConnectAttr( hDbc, SQL_ATTR_CONNECTION_DEAD, (SQLPOINTER)(&nValue), SQL_IS_UINTEGER, &nRetSize  );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors();
    }
    else
    {
        // Lets not echo out the errors here and instead assume its because the
        // hDbc was never connected or because this feature has not been implemented
        // by the driver.
        // doErrors();
        return bConnected; // just use our state
    }

    // synch our connect state with the driver state
    if ( nValue == SQL_CD_TRUE )
    {
        if ( !bConnected )
            bConnected = true;
    }
    else
    {
        if ( bConnected )
            bConnected = false;
    }
*/
    return bConnected;
}

/*!
    SQLGetInfo
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLGetInfo returns general information about the driver and data source associated with a connection.
*/
SQLRETURN MYODBCConnection::getInfo( SQLUSMALLINT nInfoType, SQLPOINTER pInfoValue, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLength )
{
    SQLRETURN nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return nReturn;
    }

    nReturn = SQLGetInfo( hDbc, nInfoType, pInfoValue, nBufferLength, pnStringLength );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
        default:
            doErrors( MYODBCMessage::MessageError );
    }

    return nReturn;
}

/*!
    getNewMessage
    
    All connection messages should be created with this. Doing so will ensure that QTMYODBCConnection
    will catch all messages and emit signal.
*/
MYODBCMessage *MYODBCConnection::getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState, char *pszMessage, SQLINTEGER nNativeCode )
{
    return new MYODBCMessage( nType, pszState, pszMessage, nNativeCode );
}

/*!
    SQLAllocHandle
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92

    SQLAllocHandle allocates an environment, connection, statement, or descriptor handle.
    Note   This function is a generic function for allocating handles that replaces the 
           ODBC 2.0 functions SQLAllocConnect, SQLAllocEnv, and SQLAllocStmt. To allow 
           applications calling SQLAllocHandle to work with ODBC 2.x drivers, a call to 
           SQLAllocHandle is mapped in the Driver Manager to SQLAllocConnect, SQLAllocEnv, 
           or SQLAllocStmt, as appropriate. For more information, see "Comments." For more 
           information about what the Driver Manager maps this function to when an ODBC 3.x 
           application is working with an ODBC 2.x driver, see "Mapping Replacement Functions 
           for Backward Compatibility of Applications" in Chapter 17: Programming 
           Considerations.
*/    
SQLRETURN MYODBCConnection::doAlloc()
{
    // SANITY CHECKS
    if ( !penvironment )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): penvironment is NULL", -1 ) );
        return SQL_ERROR;
    }
    if ( penvironment->getEnv() == SQL_NULL_HENV && !penvironment->doAlloc() )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): hEnv not allocated", -1 ) );
        return SQL_ERROR;
    }
    if ( hDbc != SQL_NULL_HDBC )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): hDbc already allocated", -1 ) );
        return SQL_ERROR;
    }

    // DO IT
    SQLRETURN nReturn;
    nReturn = SQLAllocHandle( SQL_HANDLE_DBC, penvironment->getEnv(), &hDbc );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doAlloc(): Failed to allocate connection", -1 ) );
        doErrors( MYODBCMessage::MessageError );
        return nReturn;
    }

    // set pre-connect attributes
    if ( bSQL_ATTR_ACCESS_MODE )
        setConnectAttr( SQL_ATTR_ACCESS_MODE, (SQLPOINTER)nSQL_ATTR_ACCESS_MODE, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_ASYNC_ENABLE ) 
        setConnectAttr( SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)nSQL_ATTR_ASYNC_ENABLE, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_CONNECTION_TIMEOUT )
        setConnectAttr( SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)nSQL_ATTR_CONNECTION_TIMEOUT, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_CURRENT_CATALOG )
        setConnectAttr( SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)pszSQL_ATTR_CURRENT_CATALOG, SQL_NTS );
    if ( bSQL_ATTR_LOGIN_TIMEOUT )
        setConnectAttr( SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)nSQL_ATTR_LOGIN_TIMEOUT, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_PACKET_SIZE )
        setConnectAttr( SQL_ATTR_PACKET_SIZE, (SQLPOINTER)nSQL_ATTR_PACKET_SIZE, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_TRACE )
        setConnectAttr( SQL_ATTR_TRACE, (SQLPOINTER)nSQL_ATTR_TRACE, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_TRACEFILE )
        setConnectAttr( SQL_ATTR_TRACEFILE, (SQLPOINTER)pszSQL_ATTR_TRACEFILE, SQL_NTS );

    return nReturn;
}

/*!
    SQLFreeHandle
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92
    
    SQLFreeHandle frees resources associated with a specific environment, connection, statement, 
    or descriptor handle.
    Note   This function is a generic function for freeing handles. It replaces the ODBC 2.0 
           functions SQLFreeConnect (for freeing a connection handle) and SQLFreeEnv (for freeing an 
           environment handle). SQLFreeConnect and SQLFreeEnv are both deprecated in ODBC 3.x. SQLFreeHandle 
           also replaces the ODBC 2.0 function SQLFreeStmt (with the SQL_DROP Option) for freeing a statement 
           handle. For more information, see "Comments." For more information about what the Driver Manager 
           maps this function to when an ODBC 3.x application is working with an ODBC 2.x driver, see "Mapping 
           Replacement Functions for Backward Compatibility of Applications" in Chapter 17: Programming 
           Considerations.    
*/
SQLRETURN MYODBCConnection::doFree( bool bForce )
{
    SQLRETURN nReturn;

    // SANITY CHECKS
    if ( hDbc == SQL_NULL_HDBC )
    {
        objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doFree(): hDbc not allocated", -1 ) );
        return SQL_ERROR;
    }

    // DISCONNECT
    // - All of the SQLHSTMT's for a SQLHDBC will be invalidated by ODBC upon disconnect 
    //   and/or free so call doDisconnect() to explicitly cleanup in ODBC++ before this happens.
    // - Do not use getIsConnected() because we want to do this even if the connection has 
    //   timed-out or has been otherwise lost without us knowing.
    if ( bConnected )
    {
        nReturn = doDisconnect();
        if ( !SQL_SUCCEEDED( nReturn ) && !bForce )
            return nReturn;
    }

    // DO IT
    nReturn = SQLFreeHandle( SQL_HANDLE_DBC, hDbc );

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

    hDbc = SQL_NULL_HDBC;

    return nReturn;
}

/*!
    SQLConnect
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92
    
    SQLConnect establishes connections to a driver and a data source. The connection handle 
    references storage of all information about the connection to the data source, including 
    status, transaction state, and error information.
*/
SQLRETURN MYODBCConnection::doConnect( SQLCHAR *pszDSN, SQLSMALLINT nLength1, SQLCHAR *pszUID, SQLSMALLINT nLength2, SQLCHAR *pszPWD, SQLSMALLINT nLength3 )
{
    SQLRETURN   nReturn;

    // SANITY CHECKS
    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return nReturn;
    }

    // DO IT
    nReturn = SQLConnect( hDbc, pszDSN, nLength1, pszUID, nLength2, pszPWD, nLength3 );
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
// free handle for iODBC
#ifdef Q_WS_MACX
            doFree();
            hDbc = SQL_NULL_HDBC;
#endif
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doConnect(): SQLConnect() produced an invalid return value.", -1 ) );
            return SQL_ERROR;
    }

    // Connected!
    bConnected = true;

    setPostConnectAttributes();

    return nReturn;
}

/*!
    SQLDriverConnect
    Version Introduced: ODBC 1.0
    Standards Compliance: ODBC
    
    SQLDriverConnect is an alternative to SQLConnect. It supports data sources that require 
    more connection information than the three arguments in SQLConnect, dialog boxes to 
    prompt the user for all connection information, and data sources that are not defined 
    in the system information.
    
    SQLDriverConnect provides the following connection attributes:
    
        * Establish a connection using a connection string that contains the data source 
          name, one or more user IDs, one or more passwords, and other information required 
          by the data source.
        * Establish a connection using a partial connection string or no additional 
          information; in this case, the Driver Manager and the driver can each prompt the 
          user for connection information.
        * Establish a connection to a data source that is not defined in the system 
          information. If the application supplies a partial connection string, the driver 
          can prompt the user for connection information.
        * Establish a connection to a data source using a connection string constructed 
          from the information in a .dsn file.
    
    After a connection is established, SQLDriverConnect returns the completed connection 
    string. The application can use this string for subsequent connection requests. For 
    more information, see "Connecting with SQLDriverConnect" in Chapter 6: Connecting to a 
    Data Source or Driver.
*/
SQLRETURN MYODBCConnection::doDriverConnect( SQLHWND hWnd, SQLCHAR *pszIn, SQLSMALLINT nLengthIn, SQLCHAR *pszOut, SQLSMALLINT nLengthOut, SQLSMALLINT *pnLengthOut, SQLUSMALLINT nPrompt )
{
    SQLRETURN nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return nReturn;
    }

    nReturn = SQLDriverConnect( hDbc, hWnd, pszIn, nLengthIn, pszOut, nLengthOut, pnLengthOut, nPrompt );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_NEED_DATA:
            return nReturn;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doDriverConnect(): SQLDriverConnect() produced an invalid return value.", -1 ) );
            return SQL_ERROR;
    }

    // Connected!
    bConnected = true;

    setPostConnectAttributes();

    return nReturn;
}

/*!
    SQLBrowseConnect
    Version Introduced: ODBC 1.0
    Standards Compliance: ODBC
    
    SQLBrowseConnect supports an iterative method of discovering and enumerating the 
    attributes and attribute values required to connect to a data source. Each call 
    to SQLBrowseConnect returns successive levels of attributes and attribute values. 
    When all levels have been enumerated, a connection to the data source is completed 
    and a complete connection string is returned by SQLBrowseConnect. A return code of 
    SQL_SUCCESS or SQL_SUCCESS_WITH_INFO indicates that all connection information has 
    been specified and the application is now connected to the data source.
*/
SQLRETURN MYODBCConnection::doBrowseConnect( SQLCHAR *pszInConnectionString, SQLSMALLINT nStringLength1, SQLCHAR *pszOutConnectionString, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLength2Ptr )
{
    SQLRETURN nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return nReturn;
    }

    nReturn = SQLBrowseConnect( hDbc, pszInConnectionString, nStringLength1, pszOutConnectionString, nBufferLength, pnStringLength2Ptr );
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            break;
        case SQL_SUCCESS_WITH_INFO:
            doErrors( MYODBCMessage::MessageInfo );
            break;
        case SQL_NEED_DATA:
            return nReturn;
        case SQL_ERROR:
        case SQL_INVALID_HANDLE:
            doErrors( MYODBCMessage::MessageError );
            return nReturn;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doBrowseConnect(): SQLBrowseConnect() produced an invalid return value.", -1 ) );
            return SQL_ERROR;
    }

    // Connected!
    bConnected = true;

    setPostConnectAttributes();

    return nReturn;
}

/*!
    SQLDisconnect
    Version Introduced: ODBC 1.0
    Standards Compliance: ISO 92

    SQLDisconnect closes the connection associated with a specific connection handle.
*/
SQLRETURN MYODBCConnection::doDisconnect()
{
    SQLRETURN nReturn;

    // SANITY CHECKS
    if ( !hDbc )
        return SQL_ERROR;

    // FREE SQLHSTMT's
    MYODBCObject *    pobject = objectlistZChildren.getFirst();
    MYODBCStatement * pstatement;

    while ( pobject )
    {
        pstatement = (MYODBCStatement*)pobject; // use rtti here or not bother given constructor
        if ( pstatement->getStmt() != SQL_NULL_HDBC )
        {
            if ( !SQL_SUCCEEDED( ( nReturn = pstatement->doFree() ) ) )
                return nReturn;
        }
        pobject = pobject->getZNext();
    }

    // DO IT
    nReturn = SQLDisconnect( hDbc );
    if ( nReturn == SQL_SUCCESS )
    {
    }
    else if ( nReturn == SQL_SUCCESS_WITH_INFO )
    {
        doErrors( MYODBCMessage::MessageInfo );
    }
    else
    {
        // log the error but still set state as bConnected = false
        doErrors( MYODBCMessage::MessageError );
    }

    bConnected = false;
#ifdef Q_WS_MACX
    doFree(); // free conectin handle
#endif

    // LOG STUFF HERE

    return nReturn;
}

/*!
    doErrors
    
    Capture ODBC messages.
*/
void MYODBCConnection::doErrors( MYODBCMessage::MessageTypes nType )
{
    SQLRETURN       nReturn = SQL_SUCCESS;
    SQLCHAR		    szMessage[501];
    SQLCHAR	        szState[10];
    SQLINTEGER      nNativeCode;
    SQLSMALLINT     nReturnLength;
    MYODBCMessage *   pmessage    = 0;
    SQLSMALLINT     nRecord     = 1;

    while ( SQL_SUCCEEDED( nReturn ) )
    {
        *szState    = '\0';
        *szMessage  = '\0';
        nReturn = SQLGetDiagRec( SQL_HANDLE_DBC, hDbc, nRecord, szState, &nNativeCode, szMessage, sizeof(szMessage) - 1, &nReturnLength );
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
                // pmessage = new MYODBCMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source. No data.", -1 );
                // break;
                return;
            default:
                pmessage = getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] doErrors(): Failed to get error information from driver or data source. Unknown return code from SQLGetDiagRec().", -1 );
        }

        objectlistMessages.doAppend( pmessage );
        nRecord++;
    }
}

/*!
    setPostConnectAttributes
    
    Set post-connect attributes. Some of these may have been set in doAlloc() but 
    we need to ensure that they get done upon connect (to handle disconnect/connect ).
*/
void MYODBCConnection::setPostConnectAttributes()
{
    if ( bSQL_ATTR_ASYNC_ENABLE ) 
        setConnectAttr( SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)nSQL_ATTR_ASYNC_ENABLE, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_CONNECTION_TIMEOUT )
        setConnectAttr( SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)nSQL_ATTR_CONNECTION_TIMEOUT, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_TRACE )
        setConnectAttr( SQL_ATTR_TRACE, (SQLPOINTER)nSQL_ATTR_TRACE, SQL_IS_UINTEGER );
    if ( bSQL_ATTR_TRACEFILE )
        setConnectAttr( SQL_ATTR_TRACEFILE, (SQLPOINTER)pszSQL_ATTR_TRACEFILE, SQL_NTS );
}

/*!
    SQLSetConnectAttr
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92
    
    SQLSetConnectAttr sets attributes that govern aspects of connections.    
*/
SQLRETURN MYODBCConnection::setConnectAttr( SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nStringLength )
{
    SQLRETURN nReturn = SQLSetConnectAttr( hDbc, nAttribute, pValue, nStringLength );
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
            break;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] setConnectAttr(): SQLSetConnectAttr() produced an invalid return value.", -1 ) );
            nReturn = SQL_ERROR;
    }

    return nReturn;
}

/*!
    SQLGetConnectAttr
    Version Introduced: ODBC 3.0
    Standards Compliance: ISO 92
    
    SQLGetConnectAttr returns the current setting of a connection attribute.
    Note   For more information about what the Driver Manager maps this function to 
           when an ODBC 3.x application is working with an ODBC 2.x driver, see 
           "Mapping Replacement Functions for Backward Compatibility of Applications" 
           in Chapter 17: Programming Considerations.
*/
SQLRETURN MYODBCConnection::getConnectAttr( SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength, SQLINTEGER *pnRetSize  )
{
    SQLRETURN nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return nReturn;
    }

    nReturn = SQLGetConnectAttr( hDbc, nAttribute, pValue, nLength, pnRetSize  );
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
            break;
        default:
            doErrors( MYODBCMessage::MessageError );
            objectlistMessages.doAppend( getNewMessage( MYODBCMessage::MessageError, 0, "[ODBC++] getConnectAttr(): SQLGetConnectAttr() produced an invalid return value.", -1 ) );
            nReturn = SQL_ERROR;
    }

    return nReturn;
}


