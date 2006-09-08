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

#include "MYQTODBCLogin.h"

#include <MYODBCMessage.h>

#include "MYQTODBCEnvironment.h"
#include "MYQTODBCMessageOutput.h"

MYQTODBCLogin::MYQTODBCLogin( QWidget *pwidgetParent, MYQTODBCEnvironment *penvironment )
    : QDialog( pwidgetParent, "MYQTODBCLogin", TRUE )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[%s][%d] %p\n", __FILE__, __LINE__, this );
#endif
    this->penvironment          = penvironment;

    setCaption( "Connect..." );

    // Top layout
    QVBoxLayout *playoutTop     = new QVBoxLayout( this );
    QGridLayout *playoutFields  = new QGridLayout( playoutTop, 5, 2 );
    QHBoxLayout *playoutButtons = new QHBoxLayout( playoutTop );
    pmessageoutput              = new MYQTODBCMessageOutput( this );
    pmessageoutput->hide();
    pmessageoutput->setMinimumHeight( 50 );
    setExtension( pmessageoutput );
    setOrientation( Qt::Vertical );

    // Fields
    playoutFields->setMargin( 5 );
    playoutFields->setSpacing( 5 );

    // Fields - drivers
    plabelDriver = new QLabel( "Driver", this  );
    pcomboboxDriver = new QComboBox( this );
    playoutFields->addWidget( plabelDriver, 0, 0 );
    playoutFields->addWidget( pcomboboxDriver, 0, 1 );

    // Fields - data source names
    plabelDataSourceName = new QLabel( "Data Source Name", this  );
    pcomboboxDataSourceName = new QComboBox( this );
    playoutFields->addWidget( plabelDataSourceName, 1, 0 );
    playoutFields->addWidget( pcomboboxDataSourceName, 1, 1 );

    // Fields - user id
    plabelUserID  = new QLabel( "User ID", this  );
    playoutFields->addWidget( plabelUserID, 2, 0 );
    plineeditUserID = new QLineEdit( this );
    playoutFields->addWidget( plineeditUserID, 2, 1 );
    QToolTip::add( plineeditUserID, "your User ID\nHINT: you get this from your database administrator" );
#ifndef Q_WS_WIN
    plineeditUserID->setText( ((struct passwd *)getpwuid(getuid()))->pw_name );
#endif

    // Fields - password
    plabelPassword = new QLabel( "Password", this  );
    playoutFields->addWidget( plabelPassword, 3, 0 );
    plineeditPassword = new QLineEdit( this );
    plineeditPassword->setEchoMode( QLineEdit::Password );
    playoutFields->addWidget( plineeditPassword, 3, 1 );
    QToolTip::add( plineeditPassword, "your password\nHINT: sometimes; you can leave this blank" );

    playoutFields->setRowStretch( 4, 10 );

    // Buttons
    playoutButtons->addStretch( 10 );

    // Buttons - Ok
    QPushButton *pbOk        = new QPushButton( "&Ok", this  );
    playoutButtons->addWidget( pbOk );

    // Buttons - Cancel
    QPushButton *pbCancel    = new QPushButton( "&Cancel", this  );
    playoutButtons->addWidget( pbCancel );
    pbOk->setDefault( true );
    
    // Buttons - Cancel
    QPushButton *pbMessages  = new QPushButton( "&Messages", this  );
    playoutButtons->addWidget( pbMessages );
    pbMessages->setToggleButton( true );
    
    //
#ifndef Q_WS_WIN
    plineeditPassword->setFocus();
#else
	plineeditUserID->setFocus();
#endif

    playoutTop->activate();

    connect( penvironment, SIGNAL(signalMessage(MYODBCMessage*)), pmessageoutput, SLOT(slotMessage(MYODBCMessage*)) );
    connect( pbOk, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( pbCancel, SIGNAL(clicked()), this, SLOT(reject()) );
    connect( pbMessages, SIGNAL(toggled(bool)), this, SLOT(showExtension(bool)) );

    // do these last so we catch any errors with slotMessage
    doLoadDrivers();
    doLoadDataSourceNames();
}

MYQTODBCLogin::~MYQTODBCLogin()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[%s][%d] %p\n", __FILE__, __LINE__, this );
#endif
}


void MYQTODBCLogin::setShowDriver( bool b )
{
    if ( b )
    {
        plabelDriver->show();
        pcomboboxDriver->show();
    }
    else
    {
        plabelDriver->hide();
        pcomboboxDriver->hide();
    }
}

void MYQTODBCLogin::setShowDataSourceName( bool b )
{
    if ( b )
    {
        plabelDataSourceName->show();
        pcomboboxDataSourceName->show();
    }
    else
    {
        plabelDataSourceName->hide();
        pcomboboxDataSourceName->hide();
    }
}

void MYQTODBCLogin::setShowUserID( bool b )
{
    if ( b )
    {
        plabelUserID->show();
        plineeditUserID->show();
    }
    else
    {
        plabelUserID->hide();
        plineeditUserID->hide();
    }
}

void MYQTODBCLogin::setShowPassword( bool b )
{
    if ( b )
    {
        plabelPassword->show();
        plineeditPassword->show();
    }
    else
    {
        plabelPassword->hide();
        plineeditPassword->hide();
    }
}

void MYQTODBCLogin::setDriver( const QString &string )
{
/*
    QListBoxItem *plistboxitem = pcomboboxDriver->listBox()->findItem( string );
    if ( plistboxitem )
        pcomboboxDriver->listBox()->setCurrentItem( plistboxitem );
*/
    pcomboboxDriver->setCurrentText( string );
}

void MYQTODBCLogin::setDataSourceName( const QString &string )
{
    pcomboboxDataSourceName->setCurrentText( string );
}

void MYQTODBCLogin::setUserID( const QString &string )
{
    plineeditUserID->setText( string );
}

void MYQTODBCLogin::setPassword( const QString &string )
{
    plineeditPassword->setText( string );
}

void MYQTODBCLogin::doLoadDrivers()
{
    SQLRETURN       nReturn;
    SQLCHAR         szDRV[101];
    SQLSMALLINT     nLength1;
    SQLCHAR         szAttribute[301];
    SQLSMALLINT     nLength2;

    pcomboboxDriver->insertItem( "" );

    nReturn = penvironment->getDriver( SQL_FETCH_FIRST, szDRV, sizeof(szDRV) - 1, &nLength1, szAttribute, sizeof(szAttribute) - 1, &nLength2 );
    while ( SQL_SUCCEEDED( nReturn ) )
    {
        pcomboboxDriver->insertItem( (const char*)szDRV );
        nReturn = penvironment->getDriver( SQL_FETCH_NEXT, szDRV, sizeof(szDRV) - 1, &nLength1, szAttribute, sizeof(szAttribute) - 1, &nLength2 );
    }
}

void MYQTODBCLogin::doLoadDataSourceNames()
{
    SQLRETURN       nReturn;
    SQLCHAR         szDSN[101];
    SQLSMALLINT     nLength1;
    SQLCHAR         szDescription[301];
    SQLSMALLINT     nLength2;

    pcomboboxDataSourceName->insertItem( "" );

    nReturn = penvironment->getDataSource( SQL_FETCH_FIRST, szDSN, sizeof(szDSN) - 1, &nLength1, szDescription, sizeof(szDescription) - 1, &nLength2 );
    while ( SQL_SUCCEEDED( nReturn ) )
    {
        pcomboboxDataSourceName->insertItem( (const char*)szDSN );
        nReturn = penvironment->getDataSource( SQL_FETCH_NEXT, szDSN, sizeof(szDSN) - 1, &nLength1, szDescription, sizeof(szDescription) - 1, &nLength2 );
    }
}


