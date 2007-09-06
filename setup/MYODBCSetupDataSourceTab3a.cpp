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

#include "MYODBCSetupDataSourceTab3a.h"

MYODBCSetupDataSourceTab3a::MYODBCSetupDataSourceTab3a( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    QString         stringReturnMatchingRows( tr("The client can't handle that MySQL returns the true value of affected rows. If this flag is set, MySQL returns ``found rows'' instead. You must have MySQL 3.21.14 or newer to get this to work.") );
    QString         stringAllowBigResults( tr("Don't set any packet limit for results and parameters.") );
    QString         stringUseCompressedProtocol( tr("Use the compressed client/server protocol.") );
    QString         stringChangeBIGINTColumnsToInt( tr("Change LONGLONG columns to INT columns (some applications can't handle LONGLONG).") );
    QString         stringSafe( tr("Add some extra safety checks (should not be needed but...).") );
    QString         stringEnableReconnect( tr("Enables automatic reconnect. Attention: it is strongly not recommended to set this flag for transactional operations!") );
    QString         stringAutoIncrementIsNull( tr("Turns on/off the handling of searching for the last inserted row with WHERE auto_increment_column IS NULL") );
#if QT_VERSION >= 0x040000
    QVBoxLayout *   playoutFields = new QVBoxLayout;
    setLayout( playoutFields );
#else
    QVBoxLayout *   playoutFields = new QVBoxLayout( this );
#endif
    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxReturnMatchingRows = new MYODBCSetupCheckBox( tr("Return Matching Rows"), this );
    pcheckboxReturnMatchingRows->setAssistText( stringReturnMatchingRows );
    playoutFields->addWidget( pcheckboxReturnMatchingRows );
#if QT_VERSION >= 0x040000
    pcheckboxReturnMatchingRows->setToolTip( stringReturnMatchingRows );
#else
    QToolTip::add( pcheckboxReturnMatchingRows, stringReturnMatchingRows );
#endif

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

    pcheckboxChangeBIGINTColumnsToInt = new MYODBCSetupCheckBox( tr("Change BIGINT Columns To Int"), this );
    pcheckboxChangeBIGINTColumnsToInt->setAssistText( stringChangeBIGINTColumnsToInt );
    playoutFields->addWidget( pcheckboxChangeBIGINTColumnsToInt );
#if QT_VERSION >= 0x040000
    pcheckboxChangeBIGINTColumnsToInt->setToolTip( stringChangeBIGINTColumnsToInt );
#else
    QToolTip::add( pcheckboxChangeBIGINTColumnsToInt, stringChangeBIGINTColumnsToInt );
#endif

    pcheckboxSafe = new MYODBCSetupCheckBox( tr("Safe"), this );
    pcheckboxSafe->setAssistText( stringSafe );
    playoutFields->addWidget( pcheckboxSafe );
#if QT_VERSION >= 0x040000
    pcheckboxSafe->setToolTip( stringSafe );
#else
    QToolTip::add( pcheckboxSafe, stringSafe );
#endif

    pcheckboxEnableReconnect = new MYODBCSetupCheckBox( tr("Enable Auto Reconnect"), this );
    pcheckboxEnableReconnect->setAssistText( stringEnableReconnect );
    playoutFields->addWidget( pcheckboxEnableReconnect );
#if QT_VERSION >= 0x040000
    pcheckboxEnableReconnect->setToolTip( stringEnableReconnect );
#else
    QToolTip::add( pcheckboxEnableReconnect, stringEnableReconnect );
#endif

    pcheckboxAutoIncrementIsNull = new MYODBCSetupCheckBox( tr("Enable auto_increment NULL search"), this );
    pcheckboxAutoIncrementIsNull->setAssistText( stringAutoIncrementIsNull );
    playoutFields->addWidget( pcheckboxAutoIncrementIsNull );
#if QT_VERSION >= 0x040000
    pcheckboxAutoIncrementIsNull->setToolTip( stringAutoIncrementIsNull );
#else
    QToolTip::add( pcheckboxAutoIncrementIsNull, stringAutoIncrementIsNull );
#endif

    playoutFields->addStretch( 10 );
}


