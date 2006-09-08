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

#include "MYNodeODBCSpecialColumns.h"

#include "ID16x16.xpm"

MYNodeODBCSpecialColumns::MYNodeODBCSpecialColumns( QListView *pParent, MYQTODBCConnection *pconnection, const QString &stringTable )
    : MYNode( pParent )
{
    Init( pconnection, stringTable );
}

MYNodeODBCSpecialColumns::MYNodeODBCSpecialColumns( QListViewItem *pParent, MYQTODBCConnection *pconnection, const QString &stringTable )
    : MYNode( pParent )
{
    Init( pconnection, stringTable );
}

MYNodeODBCSpecialColumns::MYNodeODBCSpecialColumns( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString &stringTable )
    : MYNode( pParent, pAfter  )
{
    Init( pconnection, stringTable );
}

MYNodeODBCSpecialColumns::~MYNodeODBCSpecialColumns()
{
    listColumns.clear();
}

void MYNodeODBCSpecialColumns::Init( MYQTODBCConnection *pconnection, const QString &stringTable )
{
    this->pconnection   = pconnection;
    this->stringTable   = stringTable;
    setText( 0, "SpecialColumns" );
    setText( 1, "" );
    setText( 2, "" );
    sortChildItems( 1, FALSE );
    listColumns.setAutoDelete( TRUE );
    this->setPixmap( 0, QPixmap( xpmID16x16 ) );
}

void MYNodeODBCSpecialColumns::setOpen( bool o )
{
    if ( o && !childCount() )
    {
        doLoadColumns();
    }
    QListViewItem::setOpen( o );
}

void MYNodeODBCSpecialColumns::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}

void MYNodeODBCSpecialColumns::doLoadColumns()
{
    MYQTODBCStatement *   pstatement = new MYQTODBCStatement( pconnection );
    SQLRETURN            nReturn;
    SQLCHAR             szColumnName[101];

    if ( !SQL_SUCCEEDED( pstatement->getSpecialColumns( SQL_BEST_ROWID, QString::null, QString::null, stringTable, SQL_SCOPE_SESSION, SQL_NULLABLE )) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        nReturn = pstatement->getData( 2, SQL_C_CHAR, szColumnName, sizeof(szColumnName), 0 );
        if ( !SQL_SUCCEEDED( nReturn ) )
            strcpy( (char *)szColumnName, "Unknown" );

        listColumns.append( new MYNodeODBCColumn( this, pconnection, (char *)szColumnName ) );
    }

    delete pstatement;
}

void MYNodeODBCSpecialColumns::selectionChanged( QListViewItem *p )
{
    MYNodeODBCColumn    *pColumn;

    for ( pColumn = listColumns.first(); pColumn != 0; pColumn = listColumns.next() )
        pColumn->selectionChanged( p );

    if ( p == this )
    {
    }
}

