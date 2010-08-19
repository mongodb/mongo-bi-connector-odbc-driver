/* Copyright (©) 2004, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/ODBC is licensed under the terms of the GPLv2
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPLv2 as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details.
   
   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MYODBCCHECKBOX_H
#define MYODBCCHECKBOX_H

#include <qcheckbox.h>
#include <qevent.h>


#ifndef MYODBC_ADD_TOOLTIP

#if QT_VERSION >= 0x040000
#    define MYODBC_ADD_TOOLTIP(control, tooltipStr) control->setToolTip(tooltipStr);
#else
#    define MYODBC_ADD_TOOLTIP(control, tooltipStr) QToolTip::add(control, tooltipStr);
#endif

#endif
class MYODBCSetupCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    MYODBCSetupCheckBox( const QString &stringText, QWidget *pwidgetParent = 0 );

    void setAssistText( const QString &stringText );

    QString getAssistText();

signals:
    void signalAssistText( const QString &stringText );

protected:
    QString stringAssistText;

    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );
};

#endif

