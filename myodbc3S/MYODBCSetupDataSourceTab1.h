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

#ifndef MYODBCUTIL_DATASOURCETAB1_H
#define MYODBCUTIL_DATASOURCETAB1_H

#include <qobject.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtooltip.h>

#include "MYODBCSetup.h"
#include "MYODBCSetupLineEdit.h"
#include "MYODBCSetupComboBoxDatabases.h"

#define MYODBC_DB_NAME_MAX 255

class MYODBCSetupDataSourceDialog;

class MYODBCSetupDataSourceTab1 : public QWidget
{
    Q_OBJECT

    friend class MYODBCSetupDataSourceDialog;
public:
    MYODBCSetupDataSourceTab1( QWidget *pwidgetParent,
                               QString stringDataSourceName,
                               QString stringDescription,
                               QString stringServer,
                               QString stringUser,
                               QString stringPassword,
                               QString stringDatabase );

    MYODBCSetupDataSourceTab1( QWidget *pwidgetParent ); 

    void setDataSourceName( const QString &stringDataSourceName );
    void setDescription( const QString &stringDescription );
    void setServer( const QString &stringServer );
    void setUser( const QString &stringUser );
    void setPassword( const QString &stringPassword );
    void setDatabase( const QString &stringDatabase );

    QString getDataSourceName();
    QString getDescription();
    QString getServer();
    QString getUser();
    QString getPassword();
    QString getDatabase();

    QLabel *plabelDataSourceName;
    QLabel *plabelDescription;
    QLabel *plabelServer;
    QLabel *plabelUser;
    QLabel *plabelPassword;
    QLabel *plabelDatabase;

    MYODBCSetupLineEdit *            plineeditDataSourceName;
    MYODBCSetupLineEdit *            plineeditDescription;
    MYODBCSetupLineEdit *            plineeditServer;
    MYODBCSetupLineEdit *            plineeditUser;
    MYODBCSetupLineEdit *            plineeditPassword;
    MYODBCSetupComboBoxDatabases *   pcomboboxDatabase;

signals:
    void signalRequestDatabaseNames();

protected:

    void doInit();
};

#endif

