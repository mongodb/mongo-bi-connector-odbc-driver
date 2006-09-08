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

#include "MYQTODBCPrompts.h"
    
MYQTODBCPrompts::MYQTODBCPrompts( QPtrList<MYQTODBCPrompt> ptrlistPrompts, QWidget *pwidgetParent )
    : QTable( pwidgetParent )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    MYQTODBCPrompt *          pprompt           = 0;
    MYQTODBCPromptTableItem * pprompttableitem  = 0;
    int                     nPrompt           = 1;
    int                     nPrompts          = ptrlistPrompts.count();
    QSettings               settings;

    this->ptrlistPrompts = ptrlistPrompts;
    setNumCols( 2 );
    setNumRows( nPrompts );

    horizontalHeader()->setLabel( 0, "Name" );
    horizontalHeader()->setLabel( 1, "Value" );

    for ( nPrompt = 0; nPrompt < nPrompts; nPrompt++ )
    {
        pprompt =  ptrlistPrompts.at( nPrompt );

        // LABEL
        setText( nPrompt, 0, pprompt->getName() );
        setItem( nPrompt, 0, 0 );

        // EDITOR
        pprompttableitem = new MYQTODBCPromptTableItem( this, QTableItem::WhenCurrent, pprompt ); 
        setText( nPrompt, 1, pprompt->getValue() );
        setItem( nPrompt, 1, pprompttableitem );
    }

    int nW = settings.readNumEntry( "/ODBC++/" + QString( className() ) + "/Col0/w", columnWidth( 0 ) );
    setColumnWidth( 0, nW );
    nW = settings.readNumEntry( "/ODBC++/" + QString( className() ) + "/Col1/w", columnWidth( 1 ) );
    setColumnWidth( 1, nW );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCPrompts::~MYQTODBCPrompts()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    QSettings settings;

    settings.writeEntry( "/ODBC++/" + QString( className() ) + "/Col0/w", columnWidth( 0 ) );
    settings.writeEntry( "/ODBC++/" + QString( className() ) + "/Col1/w", columnWidth( 1 ) );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!

*/
QWidget * MYQTODBCPrompts::createEditor( int nRow, int nCol, bool bInitFromCell ) const
{
    if ( nCol < 1 )
        return 0;

    QTableItem *i = item( nRow, nCol );
    if ( bInitFromCell || i && !i->isReplaceable() )
        return QTable::createEditor( nRow, nCol, bInitFromCell );

	return 0;
}


