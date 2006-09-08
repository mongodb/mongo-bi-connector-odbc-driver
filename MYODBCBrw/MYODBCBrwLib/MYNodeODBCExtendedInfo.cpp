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

#include "MYNodeODBCExtendedInfo.h"

MYNodeODBCExtendedInfo::MYNodeODBCExtendedInfo( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCExtendedInfo::MYNodeODBCExtendedInfo( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCExtendedInfo::MYNodeODBCExtendedInfo( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCExtendedInfo::~MYNodeODBCExtendedInfo()
{
}

void MYNodeODBCExtendedInfo::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;
    
    new MYNodeODBCEIConversion( this, pconnection ); 
    new MYNodeODBCEIDBMS( this, pconnection ); 
    new MYNodeODBCEIDataSource( this, pconnection ); 
    new MYNodeODBCEIDriver( this, pconnection ); 
    new MYNodeODBCEIFunctions( this, pconnection ); 
    new MYNodeODBCEILimits( this, pconnection ); 
    new MYNodeODBCEISupported( this, pconnection ); 
}

void MYNodeODBCExtendedInfo::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCExtendedInfo::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "Extended Info" );
    setText( 1, "" );
    setText( 2, "" );
}


