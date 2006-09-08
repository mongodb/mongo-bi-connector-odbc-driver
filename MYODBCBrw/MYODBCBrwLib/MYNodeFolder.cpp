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

#include "MYNodeFolder.h"

#include "FolderClosed16x16.xpm"
#include "FolderOpen16x16.xpm"

MYNodeFolder::MYNodeFolder( QListView *pParent )
    : MYNode( pParent )
{
    setPixmap( 0, QPixmap( xpmFolderClosed16x16 ) );
}

MYNodeFolder::MYNodeFolder( QListViewItem *pParent )
    : MYNode( pParent )
{
    setPixmap( 0, QPixmap( xpmFolderClosed16x16 ) );
}

MYNodeFolder::MYNodeFolder( QListViewItem *pParent, QListViewItem *pAfter )
    : MYNode( pParent, pAfter  )
{
    setPixmap( 0, QPixmap( xpmFolderClosed16x16 ) );
}

MYNodeFolder::~MYNodeFolder()
{
}

void MYNodeFolder::setOpen( bool bOpen )
{
    QListViewItem::setOpen( bOpen );
    listView()->setSelected( listView()->selectedItem(), false );
    if ( bOpen )
    {
        setPixmap( 0, QPixmap( xpmFolderOpen16x16 ) );
//        setSelected( true );
    }
    else
        setPixmap( 0, QPixmap( xpmFolderClosed16x16 ) );
}


