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

#include "MYQTODBCPromptDialog.h"

MYQTODBCPromptDialog::MYQTODBCPromptDialog( QWidget *pwidgetParent, QPtrList<MYQTODBCPrompt> ptrlistPrompts )
    : QDialog( pwidgetParent, "MYQTODBCPromptDialog", TRUE )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    setCaption( "QTODBC++ Connect" );

    QVBoxLayout *playoutTop = new QVBoxLayout( this );
    playoutTop->setMargin( 5 );
    
    // prompts
    pprompts = new MYQTODBCPrompts( ptrlistPrompts, this );
    playoutTop->addWidget( pprompts );

    // buttons
    QHBoxLayout *playoutButtons = new QHBoxLayout( playoutTop );
    playoutButtons->addStretch( 10 );

    QPushButton *pbOk        = new QPushButton( "&Ok", this  );
    playoutButtons->addWidget( pbOk );
    QPushButton *pbCancel    = new QPushButton( "&Cancel", this  );
    playoutButtons->addWidget( pbCancel );
    pbOk->setDefault( true );
    
//    playoutTop->activate();

    connect( pbOk, SIGNAL(clicked()), this, SLOT(slotClickedOk()) );
    connect( pbCancel, SIGNAL(clicked()), this, SLOT(slotClickedCancel()) );

    // restore geometry
    QSettings settings;
    int nWidth = settings.readNumEntry( "/ODBC++/" + QString( className() ) + "/Width", size().width() );
    int nHeight= settings.readNumEntry( "/ODBC++/" + QString( className() ) + "/Height", size().height() );
    resize( nWidth, nHeight );    

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCPromptDialog::~MYQTODBCPromptDialog()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    // save geometry
    QSettings settings;

    settings.writeEntry( "/ODBC++/" + QString( className() ) + "/Width", size().width() );
    settings.writeEntry( "/ODBC++/" + QString( className() ) + "/Height", size().height() );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

void MYQTODBCPromptDialog::slotClickedOk()
{
    accept();
}

void MYQTODBCPromptDialog::slotClickedCancel()
{
    reject();
}


