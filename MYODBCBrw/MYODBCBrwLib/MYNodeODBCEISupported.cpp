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

#include "MYNodeODBCEISupported.h"

MYNodeODBCEISupported::MYNodeODBCEISupported( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEISupported::MYNodeODBCEISupported( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEISupported::MYNodeODBCEISupported( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEISupported::~MYNodeODBCEISupported()
{
}

void MYNodeODBCEISupported::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem2;
    SQLRETURN       nReturn;
    char            szValue[1025];
    SQLUINTEGER     nBitMask;   

    // supported sql
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_PROCEDURES" );
    pnodeextendedinfoitem->setText( 2, "DBMS supports procedures" );
    nReturn = pconnection->getInfo( SQL_PROCEDURES, (SQLPOINTER)szValue, sizeof(szValue)-1, NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
        pnodeextendedinfoitem->setText( 1, szValue );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_SUBQUERIES" );
    pnodeextendedinfoitem->setText( 2, "predicates that support subqueries" );
    nReturn = pconnection->getInfo( SQL_SUBQUERIES, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_SQ_CORRELATED_SUBQUERIES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "All predicates that support subqueries support correlated subqueries." );
        if ( nBitMask & SQL_SQ_CORRELATED_SUBQUERIES )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_SQ_COMPARISON" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_SQ_COMPARISON )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_SQ_EXISTS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_SQ_EXISTS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_SQ_IN" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_SQ_IN )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_SQ_QUANTIFIED" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_SQ_QUANTIFIED )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }

}

void MYNodeODBCEISupported::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEISupported::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "Supported SQL" );
    setText( 2, "Information about the SQL statements supported by the data source." );
}


