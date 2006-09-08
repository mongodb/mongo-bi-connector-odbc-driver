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

#include "MYODBCSetupDataSourceTab1.h"

MYODBCSetupDataSourceTab1::MYODBCSetupDataSourceTab1( QWidget *pwidgetParent,
                                                      QString stringDataSourceName,
                                                      QString stringDescription,
                                                      QString stringServer,
                                                      QString stringUser,
                                                      QString stringPassword,
                                                      QString stringDatabase )
    : QWidget( pwidgetParent )
{
    doInit();
    plineeditDataSourceName->setText( stringDataSourceName );
    plineeditDescription->setText( stringDescription );
    plineeditServer->setText( stringServer );
    plineeditUser->setText( stringUser );
    plineeditPassword->setText( stringPassword );
//    pcomboboxDatabase->lineEdit()->setText( stringDatabase );
    pcomboboxDatabase->setEditText( stringDatabase );
}

MYODBCSetupDataSourceTab1::MYODBCSetupDataSourceTab1( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    doInit();
}

void MYODBCSetupDataSourceTab1::setDataSourceName( const QString &stringDataSourceName )
{
    plineeditDataSourceName->setText( stringDataSourceName );
}

void MYODBCSetupDataSourceTab1::setDescription( const QString &stringDescription )
{
    plineeditDescription->setText( stringDescription );
}

void MYODBCSetupDataSourceTab1::setServer( const QString &stringServer )
{
    plineeditServer->setText( stringServer );
}

void MYODBCSetupDataSourceTab1::setUser( const QString &stringUser )
{
    plineeditUser->setText( stringUser );
}

void MYODBCSetupDataSourceTab1::setPassword( const QString &stringPassword )
{
    plineeditPassword->setText( stringPassword );
}

void MYODBCSetupDataSourceTab1::setDatabase( const QString &stringDatabase )
{
//    pcomboboxDatabase->lineEdit()->setText( stringDatabase );
    pcomboboxDatabase->setEditText( stringDatabase );
}

QString MYODBCSetupDataSourceTab1::getDataSourceName()
{
    return plineeditDataSourceName->text();
}

QString MYODBCSetupDataSourceTab1::getDescription()
{
    return plineeditDescription->text();
}

QString MYODBCSetupDataSourceTab1::getServer()
{
    return plineeditServer->text();
}

QString MYODBCSetupDataSourceTab1::getUser()
{
    return plineeditUser->text();
}

QString MYODBCSetupDataSourceTab1::getPassword()
{
    return plineeditPassword->text();
}

QString MYODBCSetupDataSourceTab1::getDatabase()
{
//    return pcomboboxDatabase->lineEdit()->text();
    return pcomboboxDatabase->currentText();
}

void MYODBCSetupDataSourceTab1::doInit()
{
#if QT_VERSION >= 0x040000
    QGridLayout *playoutFields = new QGridLayout;
    setLayout( playoutFields );
#else
    QGridLayout *playoutFields = new QGridLayout( this );
#endif
    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );

    int nRow = 0;
    int nColLabel = 1;
    int nColField = 3;

    QString stringDataSourceName( tr("<P><B>Data Source Name (DSN)</B><HR> <P>A unique name for this data source. <P><BR><TABLE><TR><TD><B>Optional</B></TD><TD>No</TD></TR><TR><TD><B>Default</B></TD><TD>myodbc</TD></TR></TABLE>") );
    QString stringDescription( tr("<BR><P><B>Description</B><HR> <P>A brief description of this data source. <P><BR><TABLE><TR><TD><B>Optional</B></TD><TD>Yes</TD></TR><TR><TD><B>Default</B></TD><TD>[empty]</TD></TR></TABLE>") );
    QString stringServer( tr("<BR><P><B>Server</B><HR> <P>The hostname of the MySQL server.<P><BR><TABLE><TR><TD><B>Optional</B></TD><TD>Yes (silently uses default)</TD></TR><TR><TD><B>Default</B></TD><TD>localhost</TD></TR></TABLE>") );
    QString stringUser( tr("<BR><P><B>User</B><HR> <P>The username used to connect to MySQL.<P><BR><TABLE><TR><TD><B>Optional</B></TD><TD>Yes (silently uses default)</TD></TR><TR><TD><B>Default</B></TD><TD>ODBC (Windows only - otherwise [empty])</TD></TR></TABLE>") );
    QString stringPassword( tr("<BR><P><B>Password</B><HR> <P>The password for the server user combination.<P><BR><TABLE><TR><TD><B>Optional</B></TD><TD>Yes</TD></TR><TR><TD><B>Default</B></TD><TD>[empty]</TD></TR></TABLE>") );
    QString stringDatabase( tr("<BR><P><B>Database</B><HR> <P>The database to be current upon connect.<P><BR><TABLE><TR><TD><B>Optional</B></TD><TD>Yes</TD></TR><TR><TD><B>Default</B></TD><TD>[none]</TD></TR></TABLE>") );

    plabelDataSourceName = new QLabel( tr("Data Source Name"), this );
    plineeditDataSourceName = new MYODBCSetupLineEdit( this );
    plineeditDataSourceName->setAssistText( stringDataSourceName );
    playoutFields->addWidget( plabelDataSourceName, nRow, nColLabel );
    playoutFields->addWidget( plineeditDataSourceName, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditDataSourceName->setToolTip( stringDataSourceName );
#else
    QToolTip::add( plineeditDataSourceName, stringDataSourceName );
#endif
    nRow++;

    plabelDescription = new QLabel( tr("Description"), this );
    plineeditDescription = new MYODBCSetupLineEdit( this );
    plineeditDescription->setAssistText( stringDescription );
    playoutFields->addWidget( plabelDescription, nRow, nColLabel );
    playoutFields->addWidget( plineeditDescription, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditDescription->setToolTip( stringDescription );
#else
    QToolTip::add( plineeditDescription, stringDescription );
#endif
    nRow++;

    plabelServer = new QLabel( tr("Server"), this );
    plineeditServer = new MYODBCSetupLineEdit( this );
    plineeditServer->setText( "localhost" );
    plineeditServer->setAssistText( stringServer );
    playoutFields->addWidget( plabelServer, nRow, nColLabel );
    playoutFields->addWidget( plineeditServer, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditServer->setToolTip( stringServer );
#else
    QToolTip::add( plineeditServer, stringServer );
#endif
    nRow++;

    plabelUser = new QLabel( tr("User"), this );
    plineeditUser = new MYODBCSetupLineEdit( this );
    plineeditUser->setAssistText( stringUser );
    playoutFields->addWidget( plabelUser, nRow, nColLabel );
    playoutFields->addWidget( plineeditUser, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditUser->setToolTip( stringUser );
#else
    QToolTip::add( plineeditUser, stringUser );
#endif
    nRow++;

    plabelPassword = new QLabel( tr("Password"), this );
    plineeditPassword = new MYODBCSetupLineEdit( this );
    plineeditPassword->setEchoMode( QLineEdit::Password );
    plineeditPassword->setAssistText( stringPassword );
    playoutFields->addWidget( plabelPassword, nRow, nColLabel );
    playoutFields->addWidget( plineeditPassword, nRow, nColField );
#if QT_VERSION >= 0x040000
    plineeditPassword->setToolTip( stringPassword );
#else
    QToolTip::add( plineeditPassword, stringPassword );
#endif
    nRow++;

    plabelDatabase = new QLabel( tr("Database"), this );
    pcomboboxDatabase = new MYODBCSetupComboBoxDatabases( this );
    pcomboboxDatabase->setAssistText( stringDatabase );
    playoutFields->addWidget( plabelDatabase, nRow, nColLabel );
    playoutFields->addWidget( pcomboboxDatabase, nRow, nColField );
    pcomboboxDatabase->setEditable( TRUE );
    connect( pcomboboxDatabase, SIGNAL(signalLoadRequest()), SIGNAL(signalRequestDatabaseNames()) );
#if QT_VERSION >= 0x040000
    pcomboboxDatabase->setToolTip( stringDatabase );
#else
    QToolTip::add( pcomboboxDatabase, stringDatabase );
#endif
    nRow++;
}


