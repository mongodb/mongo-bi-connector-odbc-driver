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

#include "MYNodeODBCEILimits.h"

MYNodeODBCEILimits::MYNodeODBCEILimits( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEILimits::MYNodeODBCEILimits( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEILimits::MYNodeODBCEILimits( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEILimits::~MYNodeODBCEILimits()
{
}

void MYNodeODBCEILimits::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    SQLRETURN       nReturn;
    char            szValue[1025];
    SQLUSMALLINT    n;     
    SQLUINTEGER     nValue;   
    SQLSMALLINT        nLength;

    // sql limits
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_BINARY_LITERAL_LEN" );
    pnodeextendedinfoitem->setText( 2, "max length of binary literal in an SQL statement" );
    nReturn = pconnection->getInfo( SQL_MAX_BINARY_LITERAL_LEN, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( nValue ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_IDENTIFIER_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum size in characters that the data source supports for user-defined names" );
    nReturn = pconnection->getInfo( SQL_MAX_IDENTIFIER_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_CATALOG_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a catalog name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_CATALOG_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_INDEX_SIZE" );
    pnodeextendedinfoitem->setText( 2, "maximum number of bytes allowed in the combined fields of an index" );
    nReturn = pconnection->getInfo( SQL_MAX_INDEX_SIZE, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( nValue ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_CHAR_LITERAL_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a character literal in an SQL statement" );
    nReturn = pconnection->getInfo( SQL_MAX_CHAR_LITERAL_LEN, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( nValue ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_PROCEDURE_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a procedure name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_PROCEDURE_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_COLUMN_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a column name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_COLUMN_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_ROW_SIZE" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a single row in a table" );
    nReturn = pconnection->getInfo( SQL_MAX_ROW_SIZE, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( nValue ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_COLUMNS_IN_GROUP_BY" );
    pnodeextendedinfoitem->setText( 2, "maximum number of columns allowed in a GROUP BY clause" );
    nReturn = pconnection->getInfo( SQL_MAX_COLUMNS_IN_GROUP_BY, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_ROW_SIZE_INCLUDES_LONG" );
    pnodeextendedinfoitem->setText( 2, "maximum row size returned for the SQL_MAX_ROW_SIZE information type includes the length of all SQL_LONGVARCHAR and SQL_LONGVARBINARY columns in the row" );
    nReturn = pconnection->getInfo( SQL_MAX_ROW_SIZE_INCLUDES_LONG, (SQLPOINTER)szValue, sizeof(szValue)-1, &nLength );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );
    else
    {
        QString            stringMessage;
        SQLCHAR       SqlState[6], Msg[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER    NativeError;
        SQLSMALLINT   i, MsgLen;
        SQLRETURN     rc2;

       i = 1;
       while ((rc2 = SQLGetDiagRec(SQL_HANDLE_DBC, pconnection, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen)) != SQL_NO_DATA) 
       {
           stringMessage.sprintf( "[%s][%ld] %s", SqlState, NativeError, Msg );
           QMessageBox::warning( 0, "warning", stringMessage );
          i++;
       }
    }

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_COLUMNS_IN_INDEX" );
    pnodeextendedinfoitem->setText( 2, "maximum number of columns allowed in an index" );
    nReturn = pconnection->getInfo( SQL_MAX_COLUMNS_IN_INDEX, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_SCHEMA_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a schema name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_SCHEMA_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_COLUMNS_IN_ORDER_BY" );
    pnodeextendedinfoitem->setText( 2, "maximum number of columns allowed in an ORDER BY clause" );
    nReturn = pconnection->getInfo( SQL_MAX_COLUMNS_IN_ORDER_BY, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_STATEMENT_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length (number of characters, including white space) of an SQL statement" );
    nReturn = pconnection->getInfo( SQL_MAX_STATEMENT_LEN, (SQLPOINTER)&nValue, sizeof(nValue), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( nValue ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_COLUMNS_IN_SELECT" );
    pnodeextendedinfoitem->setText( 2, "maximum number of columns allowed in a select list" );
    nReturn = pconnection->getInfo( SQL_MAX_COLUMNS_IN_SELECT, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_TABLE_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a table name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_TABLE_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_COLUMNS_IN_TABLE" );
    pnodeextendedinfoitem->setText( 2, "maximum number of columns allowed in a table" );
    nReturn = pconnection->getInfo( SQL_MAX_COLUMNS_IN_TABLE, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_TABLES_IN_SELECT" );
    pnodeextendedinfoitem->setText( 2, "maximum number of tables allowed in the FROM clause of a SELECT statement" );
    nReturn = pconnection->getInfo( SQL_MAX_TABLES_IN_SELECT, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_CURSOR_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a cursor name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_CURSOR_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_MAX_USER_NAME_LEN" );
    pnodeextendedinfoitem->setText( 2, "maximum length of a user name in the data source" );
    nReturn = pconnection->getInfo( SQL_MAX_USER_NAME_LEN, (SQLPOINTER)&n, sizeof(n), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, QString::number( n ) );

}

void MYNodeODBCEILimits::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEILimits::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "SQL Limits" );
    setText( 2, "Information about the limits applied to identifiers and clauses in SQL statements, such as the maximum lengths of identifiers and the maximum number of columns in a select list. Limitations can be imposed by either the driver or the data source." );
}


