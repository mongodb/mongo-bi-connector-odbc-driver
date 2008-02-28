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
                                                      QString stringInitialStatement,
                                                      QString stringCharset,
                                                      QString stringSSLKey,
                                                      QString stringSSLCert,
                                                      QString stringSSLCA,
                                                      QString stringSSLCAPath,
                                                      QString stringSSLCipher,
                                                      QString stringSSLVerify )

    : QWidget( pwidgetParent )
{
    doInit();
    plineeditPort->setText( stringPort );
    plineeditSocket->setText( stringSocket );
    plineeditInitialStatement->setText( stringInitialStatement );
    pcomboboxCharset->setEditText( stringCharset );
    plineeditSSLKey->setText( stringSSLKey );
    plineeditSSLCert->setText( stringSSLCert );
    plineeditSSLCA->setText( stringSSLCA );
    plineeditSSLCAPath->setText( stringSSLCAPath );
    plineeditSSLCipher->setText( stringSSLCipher );
    pcheckboxSSLVerify->setChecked( stringSSLVerify == "1" ? TRUE : FALSE );
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

void MYODBCSetupDataSourceTab2::setCharset( const QString &stringCharset )
{
    pcomboboxCharset->setEditText( stringCharset );
}

void MYODBCSetupDataSourceTab2::setSSLKey( const QString &stringSSLKey )
{
    plineeditSSLKey->setText( stringSSLKey );
}

void MYODBCSetupDataSourceTab2::setSSLCert( const QString &stringSSLCert )
{
    plineeditSSLCert->setText( stringSSLCert );
}

void MYODBCSetupDataSourceTab2::setSSLCA( const QString &stringSSLCA )
{
    plineeditSSLCA->setText( stringSSLCA );
}

void MYODBCSetupDataSourceTab2::setSSLCAPath( const QString &stringSSLCAPath )
{
    plineeditSSLCAPath->setText( stringSSLCAPath );
}

void MYODBCSetupDataSourceTab2::setSSLCipher( const QString &stringSSLCipher )
{
    plineeditSSLCipher->setText( stringSSLCipher );
}

void MYODBCSetupDataSourceTab2::setSSLVerify( const QString &stringSSLVerify )
{
    pcheckboxSSLVerify->setChecked( stringSSLVerify == "1" ? TRUE : FALSE );
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

QString MYODBCSetupDataSourceTab2::getCharset()
{
    return pcomboboxCharset->currentText();
}

QString MYODBCSetupDataSourceTab2::getSSLKey()
{
    return plineeditSSLKey->text();
}

QString MYODBCSetupDataSourceTab2::getSSLCert()
{
    return plineeditSSLCert->text();
}

QString MYODBCSetupDataSourceTab2::getSSLCA()
{
    return plineeditSSLCA->text();
}

QString MYODBCSetupDataSourceTab2::getSSLCAPath()
{
    return plineeditSSLCAPath->text();
}

QString MYODBCSetupDataSourceTab2::getSSLCipher()
{
    return plineeditSSLCipher->text();
}

QString MYODBCSetupDataSourceTab2::getSSLVerify()
{
    return pcheckboxSSLVerify->isChecked() ? "1" : "";
}

void MYODBCSetupDataSourceTab2::doInit()
{
    QString         stringPort( tr("The TCP/IP port to use if server is not localhost.\nOptional: Yes (silently uses default)\nDefault: 3306") );
    QString         stringSocket( tr("The socket or Windows pipe to connect to.\nOptional: Yes\nDefault: <empty>") );
    QString         stringInitialStatement( tr("A statement that will be executed when connection to MySQL.\nOptional: Yes\nDefault: <empty>") );
    QString         stringCharset( tr("Default character set to use.\nOptional: Yes\nDefault: <empty>") );
    QString         stringSSLKey( tr("The name of the SSL key file to use for \nestablishing a secure connection.\nOptional: Yes") );
    QString         stringSSLCert( tr("The name of the SSL certificate file to use \nfor establishing a secure connection.\nOptional: Yes") );
    QString         stringSSLCA( tr("The path to a file that contains a list of \ntrusted SSL CAs.\nOptional: Yes") );
    QString         stringSSLCAPath( tr("The path to a directory that contains \ntrusted SSL CA certificates in PEM format.\nOptional: Yes") );
    QString         stringSSLCipher( tr("A list of allowable ciphers to use for SSL encryption.\nExample: ALL:-AES:-EXP\nOptional: Yes") );
    QString         stringSSLVerify( tr("The option causes the server's Common Name value in its certificate to be verified against the hostname used when connecting to the server, and the connection is rejected if there is a mismatch\nDefault: disabled") );
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

    plabel = new QLabel(tr("Character Set"), this);
    pcomboboxCharset = new MYODBCSetupComboBoxDatabases(this);
    pcomboboxCharset->setAssistText(stringCharset);
    playoutFields->addWidget(plabel, nRow, nColLabel);
    playoutFields->addWidget(pcomboboxCharset, nRow, nColField);
    pcomboboxCharset->setEditable(TRUE);
    connect(pcomboboxCharset, SIGNAL(signalLoadRequest()),
            SIGNAL(signalRequestCharsetNames()));
#if QT_VERSION >= 0x040000
    pcomboboxCharset->setToolTip(stringCharset);
#else
    QToolTip::add(pcomboboxCharset, stringCharset);
#endif
    nRow++;

    plabel = new QLabel( tr("SSL Key"), this );
    plineeditSSLKey = new MYODBCSetupLineEdit( this );
    plineeditSSLKey->setAssistText( stringSSLKey );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditSSLKey, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditSSLKey->setToolTip( stringSSLKey );
#else
    QToolTip::add( plineeditSSLKey, stringSSLKey );
#endif
    nRow++;

    plabel = new QLabel( tr("SSL Certificate"), this );
    plineeditSSLCert = new MYODBCSetupLineEdit( this );
    plineeditSSLCert->setAssistText( stringSSLCert );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditSSLCert, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditSSLCert->setToolTip( stringSSLCert );
#else
    QToolTip::add( plineeditSSLCert, stringSSLCert );
#endif
    nRow++;

    plabel = new QLabel( tr("SSL Certificate Authority"), this );
    plineeditSSLCA = new MYODBCSetupLineEdit( this );
    plineeditSSLCA->setAssistText( stringSSLCA );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditSSLCA, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditSSLCA->setToolTip( stringSSLCA );
#else
    QToolTip::add( plineeditSSLCA, stringSSLCA );
#endif
    nRow++;

    plabel = new QLabel( tr("SSL CA Path"), this );
    plineeditSSLCAPath = new MYODBCSetupLineEdit( this );
    plineeditSSLCAPath->setAssistText( stringSSLCAPath );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditSSLCAPath, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditSSLCAPath->setToolTip( stringSSLCAPath );
#else
    QToolTip::add( plineeditSSLCAPath, stringSSLCAPath );
#endif
    nRow++;

    plabel = new QLabel( tr("SSL Cipher"), this );
    plineeditSSLCipher = new MYODBCSetupLineEdit( this );
    plineeditSSLCipher->setAssistText( stringSSLCipher );
    playoutFields->addWidget( plabel, nRow, nColLabel );
    playoutFields->addWidget( plineeditSSLCipher, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditSSLCipher->setToolTip( stringSSLCipher );
#else
    QToolTip::add( plineeditSSLCipher, stringSSLCipher );
#endif
    nRow++;

    pcheckboxSSLVerify = new MYODBCSetupCheckBox( "Verify SSL Certificate", this );
    pcheckboxSSLVerify->setAssistText( stringSSLVerify );
    playoutFields->addWidget( pcheckboxSSLVerify, nRow, nColField );
#if QT_VERSION >= 0x040000
    pcheckboxSSLVerify->setToolTip( stringSSLVerify );
#else
    QToolTip::add( pcheckboxSSLVerify, stringSSLVerify );
#endif
    nRow++;
}




