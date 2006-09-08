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

#include "MYNodeODBCEIDataSource.h"

MYNodeODBCEIDataSource::MYNodeODBCEIDataSource( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIDataSource::MYNodeODBCEIDataSource( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIDataSource::MYNodeODBCEIDataSource( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEIDataSource::~MYNodeODBCEIDataSource()
{
}

void MYNodeODBCEIDataSource::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    SQLRETURN       nReturn;
    char            szValue[1025];

    // data source
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_ACCESSIBLE_PROCEDURES" );
    pnodeextendedinfoitem->setText( 2, "the user can execute all procedures returned by SQLProcedures" );
    nReturn = pconnection->getInfo( SQL_ACCESSIBLE_PROCEDURES, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_ACCESSIBLE_TABLES" );
    pnodeextendedinfoitem->setText( 2, "the user is guaranteed SELECT privileges to all tables returned by SQLTables" );
    nReturn = pconnection->getInfo( SQL_ACCESSIBLE_TABLES, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );

/*  TODO:

SQL_BOOKMARK_PERSISTENCE  
SQL_CATALOG_TERM  
SQL_COLLATION_SEQ  
SQL_CONCAT_NULL_BEHAVIOR  
SQL_CURSOR_COMMIT_BEHAVIOR  
SQL_CURSOR_ROLLBACK_BEHAVIOR  
SQL_CURSOR_SENSITIVITY  
SQL_DATA_SOURCE_READ_ONLY  
SQL_DEFAULT_TXN_ISOLATION  
SQL_DESCRIBE_PARAMETER 
SQL_MULT_RESULT_SETS
SQL_MULTIPLE_ACTIVE_TXN
SQL_NEED_LONG_DATA_LEN
SQL_NULL_COLLATION
SQL_PROCEDURE_TERM
SQL_SCHEMA_TERM
SQL_SCROLL_OPTIONS
SQL_TABLE_TERM
SQL_TXN_CAPABLE
SQL_TXN_ISOLATION_OPTION

*/

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_USER_NAME" );
    pnodeextendedinfoitem->setText( 2, "A character string with the name used in a particular database, which can be different from the login name." );
    nReturn = pconnection->getInfo( SQL_USER_NAME, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );
}

void MYNodeODBCEIDataSource::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEIDataSource::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "Data Source" );
    setText( 2, "Information about the data source, such as cursor characteristics and transaction capabilities." );
}


