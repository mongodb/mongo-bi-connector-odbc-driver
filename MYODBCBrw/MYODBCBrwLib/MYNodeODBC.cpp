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

#include "MYNodeODBC.h"

#include "ODBC16x16.xpm"
#include "Refresh16x16.xpm"
#include "Properties16x16.xpm"

MYNodeODBC::MYNodeODBC( MYListView *pParent )
    : MYNode( pParent )
{
    Init();
}

MYNodeODBC::MYNodeODBC( MYNode *pParent )
    : MYNode( pParent )
{
    Init();
}

MYNodeODBC::~MYNodeODBC()
{
    if ( penvironment )
//Double Free        delete penvironment;
    penvironment = 0;
}

void MYNodeODBC::Menu( const QPoint &point )
{
    QPopupMenu popupmenu( listView() );
    popupmenu.insertItem( QPixmap(xpmRefresh16x16), "Refresh", this, SLOT(slotLoadChildren()) );
    popupmenu.insertItem( QPixmap(xpmProperties16x16), "Properties...", this, SLOT(slotProperties()) );
    popupmenu.exec( point );
}

void MYNodeODBC::slotProperties()
{
    penvironment->doManageDataSources( listView() );
}

void MYNodeODBC::slotLoadChildren()
{
    if ( pDrivers )             delete pDrivers;
    if ( pDataSourcesSystem )   delete pDataSourcesSystem;
    if ( pDataSourcesUser )     delete pDataSourcesUser;
    pDrivers             = new MYNodeODBCDrivers( this, penvironment );
    pDataSourcesSystem    = new MYNodeODBCDataSources( this, pDrivers, SQL_FETCH_FIRST_SYSTEM, penvironment );
    pDataSourcesUser    = new MYNodeODBCDataSources( this, pDataSourcesSystem, SQL_FETCH_FIRST_USER, penvironment );
}

void MYNodeODBC::Init()
{
    pDrivers            = 0;
    pDataSourcesUser    = 0;
    pDataSourcesSystem    = 0;

    setPixmap( 0, QPixmap( xpmODBC16x16 ) );
    setText( 0, "ODBC" );
    setText( 1, "SubSystem" );
    setText( 2, "Open Database Connectivity" );

    penvironment = new MYQTODBCEnvironment();

    // echo out odbc messages
    connect( penvironment, SIGNAL(signalMessage(MYODBCMessage*)), SIGNAL(signalMessage(MYODBCMessage*)) );

    slotLoadChildren();
}



