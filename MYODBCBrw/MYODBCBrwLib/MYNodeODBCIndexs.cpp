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

#include "MYNodeODBCIndexs.h"

#include "Index16x16.xpm"

MYNodeODBCIndexs::MYNodeODBCIndexs( QListView *pParent, MYQTODBCConnection *pconnection, const QString &stringTable )
    : MYNode( pParent )
{
    Init( pconnection, stringTable );
}

MYNodeODBCIndexs::MYNodeODBCIndexs( QListViewItem *pParent, MYQTODBCConnection *pconnection, const QString &stringTable )
    : MYNode( pParent )
{
    Init( pconnection, stringTable );
}

MYNodeODBCIndexs::MYNodeODBCIndexs( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString &stringTable )
    : MYNode( pParent, pAfter )
{
    Init( pconnection, stringTable );
}

MYNodeODBCIndexs::~MYNodeODBCIndexs()
{
    listIndexs.clear();
}

void MYNodeODBCIndexs::Init( MYQTODBCConnection *pconnection, const QString &stringTable )
{
    this->pconnection   = pconnection;
    this->stringTable   = stringTable;
    setText( 0, "Indexs" );
    setText( 1, "" );
    setText( 2, "" );
    listIndexs.setAutoDelete( TRUE );
    this->setPixmap( 0, QPixmap( xpmIndex16x16 ) );
}

void MYNodeODBCIndexs::setOpen( bool o )
{
    if ( o && !childCount() )
    {
        doLoadIndexs();
    }
    QListViewItem::setOpen( o );
}

void MYNodeODBCIndexs::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}

void MYNodeODBCIndexs::doLoadIndexs()
{
    MYQTODBCStatement *   pstatement = new MYQTODBCStatement( pconnection );
    SQLRETURN            nReturn;
    SQLCHAR             szIndexName[101];
    SQLCHAR             szPrevIndexName[101];
    int                    bUnique                    = 0;
    SQLCHAR             szDesc[101];
    SQLINTEGER          nIndicator                = 0;

    if ( !SQL_SUCCEEDED( pstatement->getStatistics( QString::null, QString::null, stringTable )) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        nReturn = pstatement->getData( 4, SQL_C_LONG, &bUnique, sizeof(bUnique), &nIndicator );
        if ( nReturn != SQL_SUCCESS || nIndicator == SQL_NULL_DATA || !bUnique )
            strcpy( (char *)szDesc, "" );
        else
            strcpy( (char *)szDesc, "UNIQUE" );

        nReturn = pstatement->getData( 6, SQL_C_CHAR, szIndexName, sizeof(szIndexName), &nIndicator );
        if ( nReturn != SQL_SUCCESS || nIndicator == SQL_NULL_DATA )
            strcpy( (char *)szIndexName, "Unknown" );

        if ( strcmp( (const char*)szIndexName, (const char*)szPrevIndexName ) != 0 )
        {
            listIndexs.append( new MYNodeODBCIndex( this, pconnection, stringTable, (char *)szIndexName, (char *)szDesc ) );
            strcpy( (char*)szPrevIndexName, (char*)szIndexName );
        }
    }

    delete pstatement;
}

void MYNodeODBCIndexs::selectionChanged( QListViewItem *p )
{
    MYNodeODBCIndex    *pIndex;

    for ( pIndex = listIndexs.first(); pIndex != 0; pIndex = listIndexs.next() )
        pIndex->selectionChanged( p );

    if ( p == this )
    {
    }
}



