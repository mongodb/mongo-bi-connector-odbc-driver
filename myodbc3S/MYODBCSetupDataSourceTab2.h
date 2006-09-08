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

#ifndef MYODBCUTIL_DATASOURCETAB2_H
#define MYODBCUTIL_DATASOURCETAB2_H

#include <qobject.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>

#include "MYODBCSetup.h"
#include "MYODBCSetupLineEdit.h"

class MYODBCSetupDataSourceDialog;

class MYODBCSetupDataSourceTab2 : public QWidget
{
    Q_OBJECT

    friend class MYODBCSetupDataSourceDialog;
public:
    MYODBCSetupDataSourceTab2( QWidget *pwidgetParent,
                               QString stringPort,
                               QString stringSocket,
                               QString stringInitialStatement );

    MYODBCSetupDataSourceTab2( QWidget *pwidgetParent );

    void setPort( const QString &stringPort );
    void setSocket( const QString &stringSocket );
    void setInitialStatement( const QString &stringInitialStatement );

    QString getPort();
    QString getSocket();
    QString getInitialStatement();

protected:
    MYODBCSetupLineEdit *            plineeditPort;
    MYODBCSetupLineEdit *            plineeditSocket;
    MYODBCSetupLineEdit *            plineeditInitialStatement;

    void doInit();
};

#endif

