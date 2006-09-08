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

#include "MYNodeODBCIndex.h"

#include "Index16x16.xpm"

MYNodeODBCIndex::MYNodeODBCIndex( QListView *pParent, MYQTODBCConnection *pconnection, const QString stringTable, const QString &stringIndex, const QString &stringDesc )
    : MYNode( pParent )
{
    Init( pconnection, stringTable, stringIndex, stringDesc );
}

MYNodeODBCIndex::MYNodeODBCIndex( QListViewItem *pParent, MYQTODBCConnection *pconnection, const QString stringTable, const QString &stringIndex, const QString &stringDesc )
    : MYNode( pParent )
{
    Init( pconnection, stringTable, stringIndex, stringDesc );
}

MYNodeODBCIndex::MYNodeODBCIndex( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString stringTable, const QString &stringIndex, const QString &stringDesc )
    : MYNode( pParent, pAfter  )
{
    Init( pconnection, stringTable, stringIndex, stringDesc );
}

MYNodeODBCIndex::~MYNodeODBCIndex()
{
    listColumns.clear();
}

void MYNodeODBCIndex::Init( MYQTODBCConnection *pconnection, const QString stringTable, const QString &stringIndex, const QString &stringDesc )
{
    this->pconnection    = pconnection;
    this->stringTable    = stringTable;
    this->stringIndex    = stringIndex;
    setText( 0, stringIndex );
    setText( 1, "" );
    setText( 2, stringDesc );
    sortChildItems( 1, FALSE );
    listColumns.setAutoDelete( TRUE );
    setPixmap( 0, QPixmap( xpmIndex16x16 ) );
}

void MYNodeODBCIndex::setOpen( bool o )
{
    if ( o && !childCount() )
    {
        doLoadColumns();
    }
    QListViewItem::setOpen( o );
}

void MYNodeODBCIndex::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}

void MYNodeODBCIndex::selectionChanged( QListViewItem *p )
{
    if ( p == this )
    {
    }
}

void MYNodeODBCIndex::doLoadColumns()
{
    MYQTODBCStatement *   pstatement = new MYQTODBCStatement( pconnection );
    SQLRETURN            nReturn;
    SQLCHAR             szIndexName[101];
    SQLCHAR             szColumnName[101];
    SQLINTEGER            nIndicator;

    if ( stringTable == "" || stringIndex == "" )
    {
        delete pstatement;
        return;
    }

    if ( SQL_SUCCEEDED( pstatement->getStatistics( QString::null, QString::null, stringTable )) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        nReturn = pstatement->getData( 6, SQL_C_CHAR, szIndexName, sizeof(szIndexName), &nIndicator );
        if ( nReturn != SQL_SUCCESS || nIndicator == SQL_NULL_DATA )
            strcpy( (char *)szIndexName, "Unknown" );

        nReturn = pstatement->getData( 9, SQL_C_CHAR, szColumnName, sizeof(szColumnName), &nIndicator );
        if ( nReturn != SQL_SUCCESS || nIndicator == SQL_NULL_DATA )
            strcpy( (char *)szColumnName, "Unknown" );

        if ( strcmp( (const char*)szIndexName,  stringIndex.data() ) == 0 )
        {
            listColumns.append( new MYNodeODBCColumn( this, pconnection, (char *)szColumnName ) );
        }
    }

    delete pstatement;
}


