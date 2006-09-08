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

#include "MYODBCBrowserApp.h"

#include "DatabaseBrowse48x48.xpm"

/*!
  \class MYODBCBrowserApp MYODBCBrowserApp.h
  \brief This class is an application wrapper for MYListViewODBC. It adds all of the window 
  trimmings required to expose MYListViewODBC to the User.
  \ingroup basic

*/


/*!
  Constructs a MYODBCBrowserApp.
*/
MYODBCBrowserApp::MYODBCBrowserApp()
    : QMainWindow()
{
    setCaption( "MYODBCBrowser v" MYODBC_VERSION );
    setIcon( xpmDatabaseBrowse48x48 );

    psplitter           = new QSplitter( Qt::Vertical, this );          
    plistview           = new MYListViewODBC( psplitter );
    poutputODBC         = new MYQTODBCMessageOutput( psplitter );

    setCentralWidget( psplitter );

    connect( plistview, SIGNAL(signalMessage(ODBCMessage*)), poutputODBC, SLOT(slotMessage(ODBCMessage*)) );

    LoadState();
}

MYODBCBrowserApp::~MYODBCBrowserApp()
{
    SaveState();
}

void MYODBCBrowserApp::LoadState()
{
    QSettings settings;

    int nW = settings.readNumEntry( "/CodeByDesign/MYODBCBrowserApp/w", geometry().width() );
    int nH = settings.readNumEntry( "/CodeByDesign/MYODBCBrowserApp/h", geometry().height() );
    resize( nW, nH );

    nW = plistview->width();
    nH = settings.readNumEntry( "/CodeByDesign/MYODBCBrowserApp/ListView/h", plistview->height() );
    plistview->resize( nW, nH );
}

void MYODBCBrowserApp::SaveState()
{
    QSettings settings;

    settings.writeEntry( "/CodeByDesign/MYODBCBrowserApp/w", width() );
    settings.writeEntry( "/CodeByDesign/MYODBCBrowserApp/h", height() );
    settings.writeEntry( "/CodeByDesign/MYODBCBrowserApp/ListView/h", plistview->height() );
}



