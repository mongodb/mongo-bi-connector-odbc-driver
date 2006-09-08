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

#include "MYNodeODBCDataSource.h"

#include "Connected16x16.xpm"
#include "Disconnected16x16.xpm"

MYNodeODBCDataSource::MYNodeODBCDataSource( QListView *pParent, SQLUSMALLINT nDataSourceType, const QString &stringName, const QString &stringDesc, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent )
{
    Init( nDataSourceType, stringName, stringDesc, penvironment );
}

MYNodeODBCDataSource::MYNodeODBCDataSource( QListViewItem *pParent, SQLUSMALLINT nDataSourceType, const QString &stringName, const QString &stringDesc, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent )
{
    Init( nDataSourceType, stringName, stringDesc, penvironment );
}

MYNodeODBCDataSource::MYNodeODBCDataSource( QListViewItem *pParent,QListViewItem *pAfter, SQLUSMALLINT nDataSourceType, const QString &stringName, const QString &stringDesc, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent, pAfter  )
{
    Init( nDataSourceType, stringName, stringDesc, penvironment );
}

MYNodeODBCDataSource::~MYNodeODBCDataSource()
{
    listChildren.clear();
    if ( pconnection )
        delete pconnection;
    pconnection = 0;
}

void MYNodeODBCDataSource::Menu( const QPoint &point )
{
    QPopupMenu popupmenu( listView() );
    if ( pconnection->getIsConnected() ) 
        popupmenu.insertItem( QPixmap(xpmDisconnected16x16), "&Disconnect", this, SLOT(slotDisconnect()) );
    else
        popupmenu.insertItem( QPixmap(xpmConnected16x16), "&Connect...", this, SLOT(slotConnect()) );
    popupmenu.exec( point );
}

void MYNodeODBCDataSource::slotConnect()
{
    pconnection->doConnect( listView(), stringName );
    setOpen( true );
}

void MYNodeODBCDataSource::slotDisconnect()
{
    pconnection->doDisconnect();
}

void MYNodeODBCDataSource::slotConnected()
{
    setPixmap( 0, QPixmap( xpmConnected16x16 ) );

    listChildren.append( new MYNodeODBCExtendedInfo( this, pconnection ) );
    doLoadCatalogs();

    // If have less than 2 children than we know that we failed to find any catalogs
    // so create a dummy catalog.
    if ( listChildren.count() < 2 )
    {
        listChildren.append( new MYNodeODBCCatalog( this, NULL, pconnection, "" ) );
    }
}

void MYNodeODBCDataSource::slotDisconnected()
{
    setPixmap( 0, QPixmap( xpmDisconnected16x16 ) );
    listChildren.clear();
}

void MYNodeODBCDataSource::Init( SQLUSMALLINT nDataSourceType, const QString &stringName, const QString &stringDesc, MYQTODBCEnvironment * penvironment )
{
    this->nDataSourceType   = nDataSourceType;
    this->penvironment      = penvironment;
    this->stringName        = stringName;
    pconnection             = new MYQTODBCConnection( penvironment );

    listChildren.setAutoDelete( true );

    setText( 0, stringName );
    setText( 1, "" );
    setText( 2, stringDesc );
    setPixmap( 0, QPixmap( xpmDisconnected16x16 ) );

    pconnection->setPromptDataSourceName( false );

    connect( pconnection, SIGNAL(signalConnected()), this, SLOT(slotConnected()) );
    connect( pconnection, SIGNAL(signalDisconnected()), this, SLOT(slotDisconnected()) );
}

void MYNodeODBCDataSource::doLoadCatalogs()
{
    MYQTODBCStatement *   pstatement  = new MYQTODBCStatement( pconnection );
    SQLRETURN            nReturn;
    SQLCHAR             szName[101];
    SQLINTEGER          nIndicator;
    MYNodeODBCCatalog* pcatalog    = NULL;

    // SQL_ALL_CATALOGS only works on v3 drivers but when used on a v2 driver this 
    // should simply fail to return a result with rows and that should be ok for us.
    if ( !SQL_SUCCEEDED( pstatement->getCatalogs( SQL_ALL_CATALOGS ) ) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        *szName = '\0';
    
        nReturn = pstatement->getData( 1, SQL_C_CHAR, szName, sizeof(szName) - 1, &nIndicator );
        if ( SQL_SUCCEEDED( nReturn ) && nIndicator != SQL_NULL_DATA && *szName != '\0' )
            listChildren.append( pcatalog = new MYNodeODBCCatalog( this, pcatalog, pconnection, (const char *)szName ) );
    }

    delete pstatement;
}


