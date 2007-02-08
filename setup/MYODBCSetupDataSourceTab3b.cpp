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

#include "MYODBCSetupDataSourceTab3b.h"

MYODBCSetupDataSourceTab3b::MYODBCSetupDataSourceTab3b( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    QString         stringDontPromptOnConnect( tr("Don't prompt for questions even if driver would like to prompt.") );
    QString         stringEnableDynamicCursor( tr("Enable or disable the dynamic cursor support. (Not allowed in MyODBC 2.50.)") );
    QString         stringIgnorePoundInTable( tr("Ignore use of database name in db_name.tbl_name.col_name.") );
    QString         stringUseManagerCursors( tr("Force use of ODBC manager cursors (experimental).") );
    QString         stringDontUseSetLocale( tr("Disable the use of extended fetch (experimental).") );
    QString         stringPadCharToFullLen( tr("Pad CHAR columns to full column length.") );
    QString         stringDontCacheResults( tr("Do not cache the results locally in the driver, instead read from server (mysql_use_result()). This works only for forward-only cursors. This option is very important in dealing with large tables when you don't want the driver to cache the entire result set.") );
#if QT_VERSION >= 0x040000
    QVBoxLayout *   playoutFields = new QVBoxLayout();
    setLayout( playoutFields );
#else
    QVBoxLayout *   playoutFields = new QVBoxLayout( this );
#endif

    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxDontPromptOnConnect = new MYODBCSetupCheckBox( tr("Don't Prompt Upon Connect"), this );
    pcheckboxDontPromptOnConnect->setAssistText( stringDontPromptOnConnect );
    playoutFields->addWidget( pcheckboxDontPromptOnConnect );
#if QT_VERSION >= 0x040000
    pcheckboxDontPromptOnConnect->setToolTip( stringDontPromptOnConnect );
#else
    QToolTip::add( pcheckboxDontPromptOnConnect, stringDontPromptOnConnect );
#endif

    pcheckboxEnableDynamicCursor = new MYODBCSetupCheckBox( tr("Enable Dynamic Cursor"), this );
    pcheckboxEnableDynamicCursor->setAssistText( stringEnableDynamicCursor );
    playoutFields->addWidget( pcheckboxEnableDynamicCursor );
#if QT_VERSION >= 0x040000
    pcheckboxEnableDynamicCursor->setToolTip( stringEnableDynamicCursor );
#else
    QToolTip::add( pcheckboxEnableDynamicCursor, stringEnableDynamicCursor );
#endif

    pcheckboxIgnorePoundInTable = new MYODBCSetupCheckBox( tr("Ignore # In Table Name"), this );
    pcheckboxIgnorePoundInTable->setAssistText( stringIgnorePoundInTable );
    playoutFields->addWidget( pcheckboxIgnorePoundInTable );
#if QT_VERSION >= 0x040000
    pcheckboxIgnorePoundInTable->setToolTip( stringIgnorePoundInTable );
#else
    QToolTip::add( pcheckboxIgnorePoundInTable, stringIgnorePoundInTable );
#endif

    pcheckboxUseManagerCursors = new MYODBCSetupCheckBox( tr("User Manager Cursors"), this );
    pcheckboxUseManagerCursors->setAssistText( stringUseManagerCursors );
    playoutFields->addWidget( pcheckboxUseManagerCursors );
#if QT_VERSION >= 0x040000
    pcheckboxUseManagerCursors->setToolTip( stringUseManagerCursors );
#else
    QToolTip::add( pcheckboxUseManagerCursors, stringUseManagerCursors );
#endif

    pcheckboxDontUseSetLocale = new MYODBCSetupCheckBox( tr("Don't Use Set Locale"), this );
    pcheckboxDontUseSetLocale->setAssistText( stringDontUseSetLocale );
    playoutFields->addWidget( pcheckboxDontUseSetLocale );
#if QT_VERSION >= 0x040000
    pcheckboxDontUseSetLocale->setToolTip( stringDontUseSetLocale );
#else
    QToolTip::add( pcheckboxDontUseSetLocale, stringDontUseSetLocale );
#endif

    pcheckboxPadCharToFullLen = new MYODBCSetupCheckBox( tr("Pad Char To Full Length"), this );
    pcheckboxPadCharToFullLen->setAssistText( stringPadCharToFullLen );
    playoutFields->addWidget( pcheckboxPadCharToFullLen );
#if QT_VERSION >= 0x040000
    pcheckboxPadCharToFullLen->setToolTip( stringPadCharToFullLen );
#else
    QToolTip::add( pcheckboxPadCharToFullLen, stringPadCharToFullLen );
#endif

    pcheckboxDontCacheResults = new MYODBCSetupCheckBox( tr("Don't Cache Result (forward only cursors)"), this );
    pcheckboxDontCacheResults->setAssistText( stringDontCacheResults );
    playoutFields->addWidget( pcheckboxDontCacheResults );
#if QT_VERSION >= 0x040000
    pcheckboxDontCacheResults->setToolTip( stringDontCacheResults );
#else
    QToolTip::add( pcheckboxDontCacheResults, stringDontCacheResults );
#endif

    playoutFields->addStretch( 10 );
}


