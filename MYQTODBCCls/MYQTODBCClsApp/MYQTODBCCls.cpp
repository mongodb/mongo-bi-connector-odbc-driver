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

#include "MYQTODBCCls.h"

#include "Execute16x16.xpm"
#include "Connected16x16.xpm"
#include "Disconnected16x16.xpm"
#include "ODBC48x48.xpm"

qtodbc::qtodbc()
    : QMainWindow( 0, "qtodbc" /*, WDestructiveClose */ )
{
    setIcon( xpmODBC48x48 );

    // INIT ODBC
    penvironment    = new MYQTODBCEnvironment();
    connect( penvironment, SIGNAL(signalMessage(MYODBCMessage *)), this, SLOT(slotMessage(MYODBCMessage *)) );

    pconnection     = new MYQTODBCConnection( penvironment );
    connect( pconnection, SIGNAL(signalConnected()), this, SLOT(slotConnected()) );
    connect( pconnection, SIGNAL(signalDisconnected()), this, SLOT(slotDisconnected()) );
    
    pstatement      = new MYQTODBCStatement( pconnection );
    connect( pstatement, SIGNAL(signalResults(MYQTODBCStatement*)), this, SLOT(slotResults(MYQTODBCStatement*)) );

    // INIT TOOLBAR
    ptoolbar = new QToolBar( "toolbar", this );

    pactionConnect = new QAction( "toggle connect", QPixmap( xpmDisconnected16x16 ), "&Connect", 0, this, "QTODBCconnect" );
    pactionConnect->addTo( ptoolbar );
    connect( pactionConnect, SIGNAL(activated()), this, SLOT(slotConnectToggle()) );
    
    pactionExecute = new QAction( "execute", QPixmap( xpmExecute16x16 ), "&Execute", 0, this, "QTODBCexecute" );
    pactionExecute->addTo( ptoolbar );
    connect( pactionExecute, SIGNAL(activated()), this, SLOT(slotExecute()) );

    // INIT CLIENT AREA
    psplitter           = new QSplitter( Qt::Vertical, this );          
    ptexteditSQL        = new QTextEdit( psplitter );
    ptableResults       = new QTable( psplitter );
    ptexteditMessages   = new QTextEdit( psplitter );

    setCentralWidget( psplitter );
    resize( 450, 600 );

    penvironment->doAlloc();
    pconnection->doAlloc();

}

qtodbc::~qtodbc()
{
    if ( penvironment )
        delete penvironment; // will delete connection and statement for us
}

void qtodbc::slotConnectToggle()
{
    if ( pconnection->getIsConnected() )
        pconnection->doDisconnect();
    else
        pconnection->doConnect( this );
}

void qtodbc::slotConnected()
{
    pactionConnect->setIconSet( QPixmap( xpmConnected16x16 ) );
}

void qtodbc::slotDisconnected()
{
    pactionConnect->setIconSet( QPixmap( xpmDisconnected16x16 ) );
}

void qtodbc::slotExecute() 
{
    // clear table
    ptableResults->setNumCols( 0 );
    ptableResults->setNumRows( 0 );

    //
    pstatement->slotExecute( ptexteditSQL->text() );
}

void qtodbc::slotResults( MYQTODBCStatement * ) 
{
    doResultGUIGrid();
}

void qtodbc::slotMessage( MYODBCMessage *pmessage ) 
{
    ptexteditMessages->append( (char*)pmessage->getMessage() );
}

void qtodbc::doResultGUIGrid()
{
    SWORD       nCols;

    // GET NUMBER OF COLUMNS RETURNED
    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nCols )) )
        nCols = 0;

    if ( nCols < 0 ) nCols = 0;

    ptableResults->setNumRows( 0 );
    ptableResults->setNumCols( nCols );

    // GET A RESULTS HEADER (column headers)
    doResultGUIGridHeader( nCols );

    // GET A RESULTS BODY (data)
    if ( nCols > 0 )
        doResultGUIGridBody( nCols );
}

void qtodbc::doResultGUIGridHeader( SWORD nColumns )
{
	int			nCol;
	SQLCHAR		szColumnName[101]	= "";	
    QHeader *   pheader;
    
    pheader = ptableResults->horizontalHeader();
	for( nCol = 0; nCol < nColumns; nCol++ )
	{
		pstatement->getColAttribute( nCol+1, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), 0, 0 );
        pheader->setLabel( nCol, QString( (char*)szColumnName ) );
	}
}

void qtodbc::doResultGUIGridBody( SWORD nColumns )
{
    SQLRETURN      	nReturn             = 0;
    SQLINTEGER      nRow                = 0;
    SQLUSMALLINT    nCol            	= 0;
	SQLINTEGER      nIndicator;
	char			szColumn[300];

    // PROCESS ALL ROWS
    while( SQL_SUCCEEDED( (nReturn = pstatement->getFetch()) ) )
    {
        nRow++;
        // SOME DRIVERS DO NOT RETURN THE ROW COUNT PROPERLY SO EXPAND IF NEED BE 
        if ( ptableResults->numRows() < nRow )
            ptableResults->setNumRows( nRow );

        // PROCESS ALL COLUMNS
        for( nCol = 0; nCol < nColumns; nCol++ )
        {
            nReturn = pstatement->getData( nCol+1, SQL_C_CHAR, (SQLPOINTER)szColumn, sizeof(szColumn)-1, &nIndicator );
            if ( SQL_SUCCEEDED( nReturn ) && nIndicator != SQL_NULL_DATA )
                ptableResults->setText( nRow - 1, nCol, QString( szColumn ) );
            else if ( nReturn == SQL_ERROR )
                break;
            else
                ptableResults->setText( nRow - 1, nCol, QString( "" ) );
        }

    } // while rows
}


