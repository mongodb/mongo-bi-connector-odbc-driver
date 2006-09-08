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

#include "MYNode.h"

/*!
  \class MYNode MYNode.h
  \brief This class has been created so that we can respond to a request to 
  present a context sensitive menu and to handle event when we are double 
  clicked.
  \ingroup basic

  This class has been created to work in a MYListView and will not fully
  function if used with a QListView.
*/


/*!
  Constructs a MYNode.
*/

MYNode::MYNode( QListView *pParent )
    : QListViewItem( pParent )
{
    Init();
}

MYNode::MYNode( QListViewItem *pParent )
    : QListViewItem( pParent )
{
    Init();
}

MYNode::MYNode( QListViewItem *pParent, QListViewItem *pAfter )
    : QListViewItem( pParent, pAfter )
{
    Init();
}

MYNode::~MYNode()
{
}

void MYNode::Menu( const QPoint & )
{
    // no menu by default
}

void MYNode::HandleDoubleClicked()
{
    // default to expand/collapse node
    if ( !isOpen() ) 
    {
        if ( isExpandable() || childCount() )
            listView()->setOpen( this, TRUE );
    } 
    else 
        listView()->setOpen( this, FALSE );
}

void MYNode::Init()
{
    //    no columns by default
}





