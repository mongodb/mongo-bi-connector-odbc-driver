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

#include "MYODBCSetupDataSourceTab2.h"

MYODBCSetupDataSourceTab2::MYODBCSetupDataSourceTab2( QWidget *pwidgetParent,
                                                      QString stringPort,
                                                      QString stringSocket,
                                                      QString stringInitialStatement )
    : QWidget( pwidgetParent )
{
    doInit();
    plineeditPort->setText( stringPort );
    plineeditSocket->setText( stringSocket );
    plineeditInitialStatement->setText( stringInitialStatement );
}

MYODBCSetupDataSourceTab2::MYODBCSetupDataSourceTab2( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    doInit();
}

void MYODBCSetupDataSourceTab2::setPort( const QString &stringPort )
{
    plineeditPort->setText( stringPort );
}

void MYODBCSetupDataSourceTab2::setSocket( const QString &stringSocket )
{
    plineeditSocket->setText( stringSocket );
}

void MYODBCSetupDataSourceTab2::setInitialStatement( const QString &stringInitialStatement )
{
    plineeditInitialStatement->setText( stringInitialStatement );
}

QString MYODBCSetupDataSourceTab2::getPort()
{
    return plineeditPort->text();
}

QString MYODBCSetupDataSourceTab2::getSocket()
{
    return plineeditSocket->text();
}

QString MYODBCSetupDataSourceTab2::getInitialStatement()
{
    return plineeditInitialStatement->text();
}

void MYODBCSetupDataSourceTab2::doInit()
{
    QString         stringPort( tr("The TCP/IP port to use if server is not localhost.\nOptional: Yes (silently uses default)\nDefault: 3306") );
    QString         stringSocket( tr("The socket or Windows pipe to connect to.\nOptional: Yes\nDefault: <empty>") );
    QString         stringInitialStatement( tr("A statement that will be executed when connection to MySQL.\nOptional: Yes\nDefault: <empty>") );
#if QT_VERSION >= 0x040000
    QGridLayout *   playoutFields = new QGridLayout();
    setLayout( playoutFields );
#else
    QGridLayout *   playoutFields = new QGridLayout( this );
#endif
    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );

    QLabel *plabel;
    int nRow = 0;
    int nColLabel = 1;
    int nColField = 3;

    plabel = new QLabel( tr("Port"), this );
    plineeditPort = new MYODBCSetupLineEdit( this );
    plineeditPort->setAssistText( stringPort );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditPort, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditPort->setToolTip( stringPort );
#else
    QToolTip::add( plineeditPort, stringPort );
#endif
    nRow++;

    plabel = new QLabel( tr("Socket"), this );
    plineeditSocket = new MYODBCSetupLineEdit( this );
    plineeditSocket->setAssistText( stringSocket );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditSocket, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditSocket->setToolTip( stringSocket );
#else
    QToolTip::add( plineeditSocket, stringSocket );
#endif
    nRow++;

    plabel = new QLabel( tr("Initial Statement"), this );
    plineeditInitialStatement = new MYODBCSetupLineEdit( this );
    plineeditInitialStatement->setAssistText( stringInitialStatement );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditInitialStatement, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditInitialStatement->setToolTip( stringInitialStatement );
#else
    QToolTip::add( plineeditInitialStatement, stringInitialStatement );
#endif
    nRow++;
}


