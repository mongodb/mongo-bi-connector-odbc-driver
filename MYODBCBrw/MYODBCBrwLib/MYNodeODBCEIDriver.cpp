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

#include "MYNodeODBCEIDriver.h"

MYNodeODBCEIDriver::MYNodeODBCEIDriver( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIDriver::MYNodeODBCEIDriver( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIDriver::MYNodeODBCEIDriver( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEIDriver::~MYNodeODBCEIDriver()
{
}

void MYNodeODBCEIDriver::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem2;
    SQLRETURN       nReturn;
    char            szValue[1025];
    SQLUSMALLINT    n;     
    SQLUINTEGER     nValue;   
    SQLUINTEGER     nBitMask;   

    // driver
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_ACTIVE_ENVIRONMENTS" );
    pnodeextendedinfoitem->setText( 2, "maximum number of active environments that the driver can support" );
    nReturn = pconnection->getInfo( SQL_ACTIVE_ENVIRONMENTS, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_ASYNC_MODE" );
    pnodeextendedinfoitem->setText( 2, "the level of asynchronous support in the driver" );
    nReturn = pconnection->getInfo( SQL_DATABASE_NAME, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nValue ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_AM_CONNECTION" );
        pnodeextendedinfoitem2->setText( 1, "" );
        pnodeextendedinfoitem2->setText( 2, "Connection level asynchronous execution is supported. Either all statement handles associated with a given connection handle are in asynchronous mode or all are in synchronous mode. A statement handle on a connection cannot be in asynchronous mode while another statement handle on the same connection is in synchronous mode, and vice versa." );
        if ( nValue == SQL_AM_CONNECTION ) 
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_AM_STATEMENT" );
        pnodeextendedinfoitem2->setText( 1, "" );
        pnodeextendedinfoitem2->setText( 2, "Statement level asynchronous execution is supported. Some statement handles associated with a connection handle can be in asynchronous mode, while other statement handles on the same connection are in synchronous mode." );
        if ( nValue == SQL_AM_STATEMENT ) 
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_AM_NONE" );
        pnodeextendedinfoitem2->setText( 1, "" );
        pnodeextendedinfoitem2->setText( 2, "Asynchronous mode is not supported." );
        if ( nValue == SQL_AM_NONE )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_BATCH_ROW_COUNT" );
    pnodeextendedinfoitem->setText( 2, "behavior of the driver with respect to the availability of row counts" );
    nReturn = pconnection->getInfo( SQL_BATCH_ROW_COUNT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BRC_ROLLED_UP" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Row counts for consecutive INSERT, DELETE, or UPDATE statements are rolled up into one. If this bit is not set, then row counts are available for each individual statement." );
        if ( nBitMask & SQL_BRC_ROLLED_UP  )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BRC_PROCEDURES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Row counts, if any, are available when a batch is executed in a stored procedure. If row counts are available, they can be rolled up or individually available, depending on the SQL_BRC_ROLLED_UP bit." );
        if ( nBitMask & SQL_BRC_PROCEDURES  )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BRC_EXPLICIT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Row counts, if any, are available when a batch is executed directly by calling SQLExecute or SQLExecDirect. If row counts are available, they can be rolled up or individually available, depending on the SQL_BRC_ROLLED_UP bit." );
        if ( nBitMask & SQL_BRC_EXPLICIT  )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }
  
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_BATCH_SUPPORT" );
    pnodeextendedinfoitem->setText( 2, "the driver's support for batches" );
    nReturn = pconnection->getInfo( SQL_BATCH_SUPPORT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BS_SELECT_EXPLICIT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "The driver supports explicit batches that can have result-set generating statements." );
        if ( nBitMask & SQL_BS_SELECT_EXPLICIT )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BS_ROW_COUNT_EXPLICIT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "The driver supports explicit batches that can have row-count generating statements." );
        if ( nBitMask & SQL_BS_ROW_COUNT_EXPLICIT )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BS_SELECT_PROC" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "The driver supports explicit procedures that can have result-set generating statements." );
        if ( nBitMask & SQL_BS_SELECT_PROC )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_BS_ROW_COUNT_PROC" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "The driver supports explicit procedures that can have row-count generating statements." );
        if ( nBitMask & SQL_BS_ROW_COUNT_PROC )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }
  
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DATA_SOURCE_NAME" );
    pnodeextendedinfoitem->setText( 2, "data source name used during connection" );
    nReturn = pconnection->getInfo( SQL_DATA_SOURCE_NAME, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, szValue );
  
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DRIVER_HDBC" );
    pnodeextendedinfoitem->setText( 2, "connection handle" );
    nReturn = pconnection->getInfo( SQL_DRIVER_HDBC, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        QString stringValue;
        stringValue.sprintf( "%p", (char*)nValue );
        pnodeextendedinfoitem->setText( 1, stringValue );
    }
  
    // SQL_DRIVER_HDESC  

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DRIVER_HENV" );
    pnodeextendedinfoitem->setText( 2, "environment handle" );
    nReturn = pconnection->getInfo( SQL_DRIVER_HENV, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        QString stringValue;
        stringValue.sprintf( "%p", (char*)nValue );
        pnodeextendedinfoitem->setText( 1, stringValue );
    }
  
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DRIVER_HLIB" );
    pnodeextendedinfoitem->setText( 2, "the load library handle returned to the Driver Manager when it loaded the driver" );
    nReturn = pconnection->getInfo( SQL_DRIVER_HLIB, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        QString stringValue;
        stringValue.sprintf( "%p", (char*)nValue );
        pnodeextendedinfoitem->setText( 1, stringValue );
    }
  
    // SQL_DRIVER_HSTMT  

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DRIVER_NAME" );
    pnodeextendedinfoitem->setText( 2, "name of the ODBC driver" );
    nReturn = pconnection->getInfo( SQL_DRIVER_NAME, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, szValue );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DRIVER_ODBC_VER" );
    pnodeextendedinfoitem->setText( 2, "drivers ODBC compliance level" );
    nReturn = pconnection->getInfo( SQL_DRIVER_ODBC_VER, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, szValue );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DRIVER_VER" );
    pnodeextendedinfoitem->setText( 2, "version of driver" );
    nReturn = pconnection->getInfo( SQL_DRIVER_VER, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, szValue );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DYNAMIC_CURSOR_ATTRIBUTES1" );
    pnodeextendedinfoitem->setText( 2, "An SQLUINTEGER bitmask that describes the attributes of a static cursor that are supported by the driver. This bitmask contains the first subset of attributes; for the second subset, see SQL_STATIC_CURSOR_ATTRIBUTES2." );
    nReturn = pconnection->getInfo( SQL_DYNAMIC_CURSOR_ATTRIBUTES1, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_NEXT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A FetchOrientation argument of SQL_FETCH_NEXT is supported in a call to SQLFetchScroll when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_NEXT )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_ABSOLUTE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "FetchOrientation arguments of SQL_FETCH_FIRST, SQL_FETCH_LAST, and SQL_FETCH_ABSOLUTE are supported in a call to SQLFetchScroll when the cursor is a dynamic cursor. (The rowset that will be fetched is independent of the current cursor position.)" );
        if ( nBitMask & SQL_CA1_ABSOLUTE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_RELATIVE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "= FetchOrientation arguments of SQL_FETCH_PRIOR and SQL_FETCH_RELATIVE are supported in a call to SQLFetchScroll when the cursor is a dynamic cursor. (The rowset that will be fetched is dependent on the current cursor position. Note that this is separated from SQL_FETCH_NEXT because in a forward-only cursor, only SQL_FETCH_NEXT is supported.)" );
        if ( nBitMask & SQL_CA1_RELATIVE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A FetchOrientation argument of SQL_FETCH_BOOKMARK is supported in a call to SQLFetchScroll when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_LOCK_EXCLUSIVE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A LockType argument of SQL_LOCK_EXCLUSIVE is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_LOCK_EXCLUSIVE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_LOCK_NO_CHANGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A LockType argument of SQL_LOCK_NO_CHANGE is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_LOCK_NO_CHANGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_LOCK_UNLOCK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A LockType argument of SQL_LOCK_UNLOCK is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_LOCK_UNLOCK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_POSITION" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_POSITION is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_POS_POSITION )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_UPDATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_UPDATE is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_POS_UPDATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_DELETE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_DELETE is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_POS_DELETE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_REFRESH" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_REFRESH is supported in a call to SQLSetPos when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_POS_REFRESH )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POSITIONED_UPDATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An UPDATE WHERE CURRENT OF SQL statement is supported when the cursor is a dynamic cursor. (An SQL-92 Entry level conformant driver will always return this option as supported.)" );
        if ( nBitMask & SQL_CA1_POSITIONED_UPDATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POSITIONED_DELETE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A DELETE WHERE CURRENT OF SQL statement is supported when the cursor is a dynamic cursor. (An SQL-92 Entry level conformant driver will always return this option as supported.)" );
        if ( nBitMask & SQL_CA1_POSITIONED_DELETE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_SELECT_FOR_UPDATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A SELECT FOR UPDATE SQL statement is supported when the cursor is a dynamic cursor. (An SQL-92 Entry level conformant driver will always return this option as supported.)" );
        if ( nBitMask & SQL_CA1_SELECT_FOR_UPDATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_ADD" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_ADD is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_BULK_ADD )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_UPDATE_BY_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_UPDATE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_BULK_UPDATE_BY_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_DELETE_BY_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_DELETE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_BULK_DELETE_BY_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_FETCH_BY_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "An Operation argument of SQL_FETCH_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor." );
        if ( nBitMask & SQL_CA1_BULK_FETCH_BY_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

    }
  
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DYNAMIC_CURSOR_ATTRIBUTES2" );
    pnodeextendedinfoitem->setText( 2, "An SQLUINTEGER bitmask that describes the attributes of a static cursor that are supported by the driver. This bitmask contains the second subset of attributes; for the first subset, see SQL_STATIC_CURSOR_ATTRIBUTES1." );
    nReturn = pconnection->getInfo( SQL_DYNAMIC_CURSOR_ATTRIBUTES2, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_NEXT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_NEXT )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_ABSOLUTE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_ABSOLUTE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_RELATIVE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_RELATIVE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_LOCK_NO_CHANGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_LOCK_NO_CHANGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_LOCK_EXCLUSIVE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_LOCK_EXCLUSIVE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_LOCK_UNLOCK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_LOCK_UNLOCK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_POSITION" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_POS_POSITION )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_UPDATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_POS_UPDATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_DELETE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_POS_DELETE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POS_REFRESH" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_POS_REFRESH )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POSITIONED_UPDATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_POSITIONED_UPDATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_POSITIONED_DELETE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_POSITIONED_DELETE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_SELECT_FOR_UPDATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_SELECT_FOR_UPDATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_ADD" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_BULK_ADD )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_UPDATE_BY_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_BULK_UPDATE_BY_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_DELETE_BY_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_BULK_DELETE_BY_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_CA1_BULK_FETCH_BY_BOOKMARK" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_CA1_BULK_FETCH_BY_BOOKMARK )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }


/* TODO:
SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1  
SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2  
SQL_FILE_USAGE 
SQL_GETDATA_EXTENSIONS
*/


    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_INFO_SCHEMA_VIEWS" );
    pnodeextendedinfoitem->setText( 2, "An SQLUINTEGER bitmask enumerating the views in the INFORMATION_SCHEMA that are supported by the driver. The views in, and the contents of, INFORMATION_SCHEMA are as defined in SQL-92." );
    nReturn = pconnection->getInfo( SQL_INFO_SCHEMA_VIEWS, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_ASSERTIONS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the catalog's assertions that are owned by a given user. (Full level)" );
        if ( nBitMask & SQL_ISV_ASSERTIONS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_CHARACTER_SETS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the catalog's character sets that are accessible to a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_CHARACTER_SETS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_CHECK_CONSTRAINTS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the CHECK constraints that are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_CHECK_CONSTRAINTS )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_COLLATIONS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the character collations for the catalog that are accessible to a given user. (Full level)" );
        if ( nBitMask & SQL_ISV_COLLATIONS )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_COLUMN_DOMAIN_USAGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies columns for the catalog that are dependent on domains defined in the catalog and are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_COLUMN_DOMAIN_USAGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_COLUMN_PRIVILEGES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the privileges on columns of persistent tables that are available to or granted by a given user. (FIPS Transitional level)" );
        if ( nBitMask & SQL_ISV_COLUMN_PRIVILEGES )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_COLUMNS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the columns of persistent tables that are accessible to a given user. (FIPS Transitional level)" );
        if ( nBitMask & SQL_ISV_COLUMNS )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_CONSTRAINT_COLUMN_USAGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Similar to CONSTRAINT_TABLE_USAGE view, columns are identified for the various constraints that are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_CONSTRAINT_COLUMN_USAGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_CONSTRAINT_TABLE_USAGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the tables that are used by constraints (referential, unique, and assertions), and are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_CONSTRAINT_TABLE_USAGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_DOMAIN_CONSTRAINTS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the domain constraints (of the domains in the catalog) that are accessible to a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_DOMAIN_CONSTRAINTS )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_DOMAINS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the domains defined in a catalog that are accessible to the user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_DOMAINS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_KEY_COLUMN_USAGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies columns defined in the catalog that are constrained as keys by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_KEY_COLUMN_USAGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_REFERENTIAL_CONSTRAINTS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the referential constraints that are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_REFERENTIAL_CONSTRAINTS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_SCHEMATA" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the schemas that are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_SCHEMATA )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_SQL_LANGUAGES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the SQL conformance levels, options, and dialects supported by the SQL implementation. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_SQL_LANGUAGES )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_TABLE_CONSTRAINTS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the table constraints that are owned by a given user. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_TABLE_CONSTRAINTS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_TABLE_PRIVILEGES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the privileges on persistent tables that are available to or granted by a given user. (FIPS Transitional level)" );
        if ( nBitMask & SQL_ISV_TABLE_PRIVILEGES )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_TABLES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the persistent tables defined in a catalog that are accessible to a given user. (FIPS Transitional level)" );
        if ( nBitMask & SQL_ISV_TABLES )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_TRANSLATIONS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies character translations for the catalog that are accessible to a given user. (Full level)" );
        if ( nBitMask & SQL_ISV_TRANSLATIONS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_USAGE_PRIVILEGES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the USAGE privileges on catalog objects that are available to or owned by a given user. (FIPS Transitional level)" );
        if ( nBitMask & SQL_ISV_USAGE_PRIVILEGES )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_VIEW_COLUMN_USAGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the columns on which the catalog's views that are owned by a given user are dependent. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_VIEW_COLUMN_USAGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_VIEW_TABLE_USAGE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the tables on which the catalog's views that are owned by a given user are dependent. (Intermediate level)" );
        if ( nBitMask & SQL_ISV_VIEW_TABLE_USAGE )
            pnodeextendedinfoitem2->setText( 1, "Y" );
 
        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_ISV_VIEWS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "Identifies the viewed tables defined in this catalog that are accessible to a given user. (FIPS Transitional level)" );
        if ( nBitMask & SQL_ISV_VIEWS )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }

/* TODO:
SQL_KEYSET_CURSOR_ATTRIBUTES1
SQL_KEYSET_CURSOR_ATTRIBUTES2
SQL_MAX_ASYNC_CONCURRENT_STATEMENTS
SQL_MAX_CONCURRENT_ACTIVITIES
SQL_MAX_DRIVER_CONNECTIONS

SQL_ODBC_STANDARD_CLI_CONFORMANCE // this is probably not valid
*/

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_ODBC_INTERFACE_CONFORMANCE" );
    pnodeextendedinfoitem->setText( 2, "An SQLUINTEGER value indicating the level of the ODBC 3.x interface that the driver conforms to." );
    nReturn = pconnection->getInfo( SQL_ODBC_INTERFACE_CONFORMANCE, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        QString stringValue;
        stringValue.sprintf( "%p", (char*)nValue );
        pnodeextendedinfoitem->setText( 1, stringValue );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_OIC_CORE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "The minimum level that all ODBC drivers are expected to conform to. This level includes basic interface elements such as connection functions, functions for preparing and executing an SQL statement, basic result set metadata functions, basic catalog functions, and so on." );
        if ( nValue == SQL_OIC_CORE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_OIC_LEVEL1" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A level including the core standards compliance level functionality, plus scrollable cursors, bookmarks, positioned updates and deletes, and so on." );
        if ( nValue == SQL_OIC_LEVEL1 )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_OIC_LEVEL2" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "A level including level 1 standards compliance level functionality, plus advanced features such as sensitive cursors; update, delete, and refresh by bookmarks; stored procedure support; catalog functions for primary and foreign keys; multicatalog support; and so on." );
        if ( nValue == SQL_OIC_LEVEL2 )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_ODBC_VER" );
    pnodeextendedinfoitem->setText( 2, "ODBC sub-system version" );
    nReturn = pconnection->getInfo( SQL_ODBC_VER, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, szValue );

/* TODO:
SQL_PARAM_ARRAY_ROW_COUNTS
SQL_PARAM_ARRAY_SELECTS
SQL_ROW_UPDATES
SQL_SEARCH_PATTERN_ESCAPE
*/   
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_SERVER_NAME" );
    pnodeextendedinfoitem->setText( 2, "name of the server" );
    nReturn = pconnection->getInfo( SQL_SERVER_NAME, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        pnodeextendedinfoitem->setText( 1, szValue );

/* TODO:
SQL_STATIC_CURSOR_ATTRIBUTES1
SQL_STATIC_CURSOR_ATTRIBUTES2
*/
    
}

void MYNodeODBCEIDriver::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEIDriver::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "Driver" );
    setText( 2, "" );
}


