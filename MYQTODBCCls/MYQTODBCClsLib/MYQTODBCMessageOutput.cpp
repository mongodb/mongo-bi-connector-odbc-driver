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

#include "MYQTODBCMessageOutput.h"

#include <MYODBCMessage.h>

#include "Warning16x16.xpm"
#include "Error16x16.xpm"

MYQTODBCMessageOutput::MYQTODBCMessageOutput( QWidget *pwidgetParent )
    : QTable( 0, 4, pwidgetParent )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    setLeftMargin( 0 );
    horizontalHeader()->setLabel( 0, "" );
    horizontalHeader()->setLabel( 1, "SQLSTATE" );
    horizontalHeader()->setLabel( 2, "Native Code" );
    horizontalHeader()->setLabel( 3, "Message" );

    setColumnWidth( 0, 18 );
    setColumnWidth( 1, 20 );
    setColumnWidth( 2, 20 );

    setColumnStretchable( 3, true );

    nWaitingMessages = 0;

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCMessageOutput::~MYQTODBCMessageOutput()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif


#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

void MYQTODBCMessageOutput::setWaitingMessages( int n ) 
{ 
    nWaitingMessages = n; 
    emit signalWaitingMessages( this );
}


/*!
    slotMessage
    
    Append the message to our list of messages.
*/
void MYQTODBCMessageOutput::slotMessage( MYODBCMessage *pmessage )
{
    int nRows = this->numRows() + 1;
    int nRow  = nRows - 1;

    // add row
    setNumRows( nRows );

    // column 1
    switch ( pmessage->getType() )
    {
        case MYODBCMessage::MessageInfo: //warning really
            setPixmap( nRow, 0, xpmWarning16x16 );
            break;
        case MYODBCMessage::MessageError:
            setPixmap( nRow, 0, xpmError16x16 );
            break;
    }

    // column 2
    setText( nRow, 1, (const char *)pmessage->getState() );

    // column 3
    setText( nRow, 2, QString::number( pmessage->getNativeCode() ) );

    // column 4
    setText( nRow, 3, (const char*)pmessage->getMessage() );
    item( nRow, 3 )->setWordWrap( true );

    adjustRow( nRow );
    // setRowStretchable( nRow, true );
    ensureCellVisible( nRow, 0 );

    nWaitingMessages++;
    emit signalWaitingMessages( this );
}

