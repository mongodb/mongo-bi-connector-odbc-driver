/* Copyright 2004-2007 MySQL AB

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

#include "MYODBCSetupDataSourceTab3c.h"

MYODBCSetupDataSourceTab3c::MYODBCSetupDataSourceTab3c( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    QString      stringEnableDynamicCursor( tr("Enable or disable the dynamic cursor support. (Not allowed in MyODBC 2.50.)") );
    QString      stringUseManagerCursors( tr("Force use of ODBC manager cursors (experimental).") );
    QString      stringDontCacheResults( tr("Do not cache the results locally in the driver, instead read from server (mysql_use_result()). This works only for forward-only cursors. This option is very important in dealing with large tables when you don't want the driver to cache the entire result set.") );
    QString      stringForceUseOfForwardOnlyCursors( tr("Force the use of Forward-only cursor type. In case of applications setting the default static/dynamic cursor type, and one wants driver to use non-cache result sets, then this option will ensure the forward-only cursor behavior.") );
    QString      stringReturnMatchingRows( tr("The client can't handle that MySQL returns the true value of affected rows. If this flag is set, MySQL returns ``found rows'' instead. You must have MySQL 3.21.14 or newer to get this to work.") );
    QString      stringAutoIncrementIsNull( tr("Turns on/off the handling of searching for the last inserted row with WHERE auto_increment_column IS NULL") );
    QString      stringPadCharToFullLen( tr("Pad CHAR columns to full column length.") );
    QString      stringZeroDate2Min( tr("Return SQL_NULL_DATA for zero date.") );

#if QT_VERSION >= 0x040000
    QVBoxLayout *playoutFields = new QVBoxLayout;
    setLayout( playoutFields );
#else
    QVBoxLayout *playoutFields = new QVBoxLayout( this );
#endif

    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxEnableDynamicCursor = new MYODBCSetupCheckBox( tr("Enable Dynamic Cursor"), this );
    pcheckboxEnableDynamicCursor->setAssistText( stringEnableDynamicCursor );
    playoutFields->addWidget( pcheckboxEnableDynamicCursor );
#if QT_VERSION >= 0x040000
    pcheckboxEnableDynamicCursor->setToolTip( stringEnableDynamicCursor );
#else
    QToolTip::add( pcheckboxEnableDynamicCursor, stringEnableDynamicCursor );
#endif

    pcheckboxUseManagerCursors = new MYODBCSetupCheckBox( tr("Use Manager Cursors"), this );
    pcheckboxUseManagerCursors->setAssistText( stringUseManagerCursors );
    playoutFields->addWidget( pcheckboxUseManagerCursors );
#if QT_VERSION >= 0x040000
    pcheckboxUseManagerCursors->setToolTip( stringUseManagerCursors );
#else
    QToolTip::add( pcheckboxUseManagerCursors, stringUseManagerCursors );
#endif

    pcheckboxDontCacheResults = new MYODBCSetupCheckBox( tr("Don't Cache Result (forward only cursors)"), this );
    pcheckboxDontCacheResults->setAssistText( stringDontCacheResults );
    playoutFields->addWidget( pcheckboxDontCacheResults );
#if QT_VERSION >= 0x040000
    pcheckboxDontCacheResults->setToolTip( stringDontCacheResults );
#else
    QToolTip::add( pcheckboxDontCacheResults, stringDontCacheResults );
#endif

    pcheckboxForceUseOfForwardOnlyCursors = new MYODBCSetupCheckBox( tr("Force Use Of Forward Only Cursors"), this );
    pcheckboxForceUseOfForwardOnlyCursors->setAssistText( stringForceUseOfForwardOnlyCursors );
    playoutFields->addWidget( pcheckboxForceUseOfForwardOnlyCursors );
#if QT_VERSION >= 0x040000
    pcheckboxForceUseOfForwardOnlyCursors->setToolTip( stringForceUseOfForwardOnlyCursors );
#else
    QToolTip::add( pcheckboxForceUseOfForwardOnlyCursors, stringForceUseOfForwardOnlyCursors );
#endif

    pcheckboxReturnMatchingRows = new MYODBCSetupCheckBox( tr("Return Matching Rows"), this );
    pcheckboxReturnMatchingRows->setAssistText( stringReturnMatchingRows );
    playoutFields->addWidget( pcheckboxReturnMatchingRows );
#if QT_VERSION >= 0x040000
    pcheckboxReturnMatchingRows->setToolTip( stringReturnMatchingRows );
#else
    QToolTip::add( pcheckboxReturnMatchingRows, stringReturnMatchingRows );
#endif

    pcheckboxAutoIncrementIsNull = new MYODBCSetupCheckBox( tr("Enable auto_increment NULL search"), this );
    pcheckboxAutoIncrementIsNull->setAssistText( stringAutoIncrementIsNull );
    playoutFields->addWidget( pcheckboxAutoIncrementIsNull );
#if QT_VERSION >= 0x040000
    pcheckboxAutoIncrementIsNull->setToolTip( stringAutoIncrementIsNull );
#else
    QToolTip::add( pcheckboxAutoIncrementIsNull, stringAutoIncrementIsNull );
#endif

    pcheckboxPadCharToFullLen = new MYODBCSetupCheckBox( tr("Pad Char To Full Length"), this );
    pcheckboxPadCharToFullLen->setAssistText( stringPadCharToFullLen );
    playoutFields->addWidget( pcheckboxPadCharToFullLen );
#if QT_VERSION >= 0x040000
    pcheckboxPadCharToFullLen->setToolTip( stringPadCharToFullLen );
#else
    QToolTip::add( pcheckboxPadCharToFullLen, stringPadCharToFullLen );
#endif

    pcheckboxZeroDate2Min = new MYODBCSetupCheckBox( tr("Return SQL_NULL_DATA for zero date"), this );
    pcheckboxZeroDate2Min->setAssistText( stringZeroDate2Min );
    playoutFields->addWidget( pcheckboxZeroDate2Min );
    MYODBC_ADD_TOOLTIP(pcheckboxZeroDate2Min, stringZeroDate2Min);

    playoutFields->addStretch( 10 );
}


