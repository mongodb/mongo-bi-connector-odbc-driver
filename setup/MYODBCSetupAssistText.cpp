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

#include "MYODBCSetupAssistText.h"

MYODBCSetupAssistText::MYODBCSetupAssistText( QWidget *pwidgetParent )
    : QTextBrowser( pwidgetParent )
{
}

void MYODBCSetupAssistText::setDefaultHtml( const QString &stringHtml )
{
    stringDefaultHtml = stringHtml;
#if QT_VERSION >= 0x040000
    if ( toPlainText().isEmpty() )
        QTextBrowser::setHtml( stringDefaultHtml );
#else
    if ( text().isEmpty() )
        QTextBrowser::setText( stringDefaultHtml );
#endif
}

void MYODBCSetupAssistText::setHtml( const QString &stringHtml )
{
#if QT_VERSION >= 0x040000
    if ( stringHtml.isEmpty() )
        QTextBrowser::setHtml( stringDefaultHtml );
    else
        QTextBrowser::setHtml( stringHtml );
#else
    if ( stringHtml.isEmpty() )
        setText( stringDefaultHtml );
    else
        setText( stringHtml );
#endif
}


