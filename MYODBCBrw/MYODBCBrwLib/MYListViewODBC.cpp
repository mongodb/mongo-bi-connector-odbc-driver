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

#include "MYListViewODBC.h"

/*!
  \class MYListViewODBC MYListViewODBC.h
  \brief This class is a high-level widget which allows a user to browse and manage
  ODBC.
  \ingroup basic

*/


/*!
  Constructs a MYListViewODBC.
*/

MYListViewODBC::MYListViewODBC( QWidget *pwidgetParent )
    : MYListView( pwidgetParent )
{
    setRootIsDecorated( true );
//    setShowSortIndicator( true );
    setSelectionMode( QListView::Single );
    setAllColumnsShowFocus( true );
    addColumn( "Name" );
    addColumn( "Type" );
    addColumn( "Description" );

    pnodeodbc = new MYNodeODBC( this ); // this is always our top level node
    
    // echo out odbc messages
    connect( pnodeodbc, SIGNAL(signalMessage(MYODBCMessage*)), SIGNAL(signalMessage(MYODBCMessage*)) );
}

MYListViewODBC::~MYListViewODBC()
{
}



