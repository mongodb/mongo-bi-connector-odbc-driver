/* Copyright (©) 2004, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/ODBC is licensed under the terms of the
   GPL, like most MySQL Connectors. There are special exceptions
   to the terms and conditions of the GPL as it is applied to
   this software, see the FLOSS License Exception available on
   mysql.com.

   This program is free software; you can redistribute it and/or modify
   it under the terms of version 2 of the GNU General Public License as
   published by the Free Software Foundation.

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

#include "MYODBCSetupDataSourceTab3a.h"

MYODBCSetupDataSourceTab3a::MYODBCSetupDataSourceTab3a( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{

    QString         stringAllowBigResults( tr("Don't set any packet limit for results and parameters.") );
    QString         stringUseCompressedProtocol( tr("Use the compressed client/server protocol.") );
    QString         stringEnableReconnect( tr("Enables automatic reconnect. Attention: it is strongly not recommended to set this flag for transactional operations!") );
    QString         stringDontPromptOnConnect( tr("Don't prompt for questions even if driver would like to prompt.") );
    QString         stringForceUseOfNamedPipes( tr("Connect with named pipes to a mysqld server running on NT.") );
    QString         stringMultiStatements( tr("Allow multiple statements in a single query.") );
    QString         stringInteractive( tr("Identify connection as an interactive session") );

#if QT_VERSION >= 0x040000
    QVBoxLayout *   playoutFields = new QVBoxLayout;
    setLayout( playoutFields );
#else
    QVBoxLayout *   playoutFields = new QVBoxLayout( this );
#endif
    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxAllowBigResults = new MYODBCSetupCheckBox( tr("Allow Big Results"), this );
    pcheckboxAllowBigResults->setAssistText( stringAllowBigResults );
    playoutFields->addWidget( pcheckboxAllowBigResults );
#if QT_VERSION >= 0x040000
    pcheckboxAllowBigResults->setToolTip( stringAllowBigResults );
#else
    QToolTip::add( pcheckboxAllowBigResults, stringAllowBigResults );
#endif

    pcheckboxUseCompressedProtocol = new MYODBCSetupCheckBox( tr("Use Compressed Protocol"), this );
    pcheckboxUseCompressedProtocol->setAssistText( stringUseCompressedProtocol );
    playoutFields->addWidget( pcheckboxUseCompressedProtocol );
#if QT_VERSION >= 0x040000
    pcheckboxUseCompressedProtocol->setToolTip( stringUseCompressedProtocol );
#else
    QToolTip::add( pcheckboxUseCompressedProtocol, stringUseCompressedProtocol );
#endif

    pcheckboxEnableReconnect = new MYODBCSetupCheckBox( tr("Enable Auto Reconnect"), this );
    pcheckboxEnableReconnect->setAssistText( stringEnableReconnect );
    playoutFields->addWidget( pcheckboxEnableReconnect );
#if QT_VERSION >= 0x040000
    pcheckboxEnableReconnect->setToolTip( stringEnableReconnect );
#else
    QToolTip::add( pcheckboxEnableReconnect, stringEnableReconnect );
#endif

    pcheckboxDontPromptOnConnect = new MYODBCSetupCheckBox( tr("Don't Prompt Upon Connect"), this );
    pcheckboxDontPromptOnConnect->setAssistText( stringDontPromptOnConnect );
    playoutFields->addWidget( pcheckboxDontPromptOnConnect );
#if QT_VERSION >= 0x040000
    pcheckboxDontPromptOnConnect->setToolTip( stringDontPromptOnConnect );
#else
    QToolTip::add( pcheckboxDontPromptOnConnect, stringDontPromptOnConnect );
#endif

    pcheckboxForceUseOfNamedPipes = new MYODBCSetupCheckBox( tr("Force Use Of Named Pipes"), this );
    pcheckboxForceUseOfNamedPipes->setAssistText( stringForceUseOfNamedPipes );
    playoutFields->addWidget( pcheckboxForceUseOfNamedPipes );
#if QT_VERSION >= 0x040000
    pcheckboxForceUseOfNamedPipes->setToolTip( stringForceUseOfNamedPipes );
#else
    QToolTip::add( pcheckboxForceUseOfNamedPipes, stringForceUseOfNamedPipes );
#endif

    pcheckboxMultiStatements = new MYODBCSetupCheckBox( tr("Allow multiple statements"), this );
    pcheckboxMultiStatements->setAssistText( stringMultiStatements );
    playoutFields->addWidget( pcheckboxMultiStatements );
#if QT_VERSION >= 0x040000
    pcheckboxMultiStatements->setToolTip( stringMultiStatements );
#else
    QToolTip::add( pcheckboxMultiStatements, stringMultiStatements );
#endif

    pcheckboxInteractive = new MYODBCSetupCheckBox( tr("Interactive client"), this );
    pcheckboxInteractive->setAssistText( stringInteractive );
    playoutFields->addWidget( pcheckboxInteractive );
    MYODBC_ADD_TOOLTIP(pcheckboxInteractive, stringInteractive);

    playoutFields->addStretch( 10 );
}


