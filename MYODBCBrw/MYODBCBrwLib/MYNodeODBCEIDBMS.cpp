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

#include "MYNodeODBCEIDBMS.h"

MYNodeODBCEIDBMS::MYNodeODBCEIDBMS( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIDBMS::MYNodeODBCEIDBMS( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIDBMS::MYNodeODBCEIDBMS( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEIDBMS::~MYNodeODBCEIDBMS()
{
}

void MYNodeODBCEIDBMS::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    SQLRETURN       nReturn;
    char            szValue[1025];

    // dbms
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DATABASE_NAME" );
    pnodeextendedinfoitem->setText( 2, "current database in use" );
    nReturn = pconnection->getInfo( SQL_DATABASE_NAME, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );
        
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DBMS_NAME" );
    pnodeextendedinfoitem->setText( 2, "name of database management system" );
    nReturn = pconnection->getInfo( SQL_DBMS_NAME, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );
    
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_DBMS_VER" );
    pnodeextendedinfoitem->setText( 2, "version of database management system" );
    nReturn = pconnection->getInfo( SQL_DBMS_VER, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );
}

void MYNodeODBCEIDBMS::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEIDBMS::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "DBMS" );
    setText( 2, "Information about the DBMS product, such as the DBMS name and version." );
}


