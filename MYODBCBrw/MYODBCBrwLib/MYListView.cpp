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

#include "MYListView.h"

/*!
  \class MYListView MYListView.h
  \brief This class has been created so that we can capture the double click event and pass 
  it on to the appropriate list view item. All list view items must be MYNode class else bad 
  things will happen.
  \ingroup basic

*/


/*!
  Constructs a MYListView.
*/

MYListView::MYListView( QWidget *pwidgetParent )
    : QListView( pwidgetParent )
{
    connect( this, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)),
             SLOT(slotContextMenu(QListViewItem *, const QPoint &, int)) );
}

MYListView::~MYListView()
{
}

void MYListView::slotContextMenu( QListViewItem *plistviewitem, const QPoint &point, int )
{
    if ( !plistviewitem )
        return;
 
    MYNode *pnode = (MYNode*)plistviewitem;
    pnode->Menu( point );
}

void MYListView::contentsMouseDoubleClickEvent( QMouseEvent *pmouseevent )
{
    QPoint point = contentsToViewport( pmouseevent->pos() );
 
    MYNode *plistviewitem = (MYNode*)itemAt( point );
    if ( !plistviewitem || !plistviewitem->isEnabled() )
        return;

    plistviewitem->HandleDoubleClicked();

    emit doubleClicked( plistviewitem );
}


