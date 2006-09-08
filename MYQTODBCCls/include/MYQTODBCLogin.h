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

#ifndef MYQTODBCLOGIN_H
#define MYQTODBCLOGIN_H

//
#include <sys/types.h>

// Qt
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qtooltip.h>

//
#ifndef Q_WS_WIN
#include <unistd.h>
#include <pwd.h>
#endif

//
class MYQTODBCEnvironment;
class MYODBCMessage;
class MYQTODBCMessageOutput;

class MYQTODBCLogin: public QDialog
{
	Q_OBJECT
public:
    MYQTODBCLogin( QWidget *pwidgetParent, MYQTODBCEnvironment *penvironment );
    ~MYQTODBCLogin();

    // SETTERS
    virtual void setShowDriver( bool b );
    virtual void setShowDataSourceName( bool b );
    virtual void setShowUserID( bool b );
    virtual void setShowPassword( bool b );
    virtual void setDriver( const QString &string );
    virtual void setDataSourceName( const QString &string );
    virtual void setUserID( const QString &string );
    virtual void setPassword( const QString &string );

    // GETTERS
    QString getDriver() { return pcomboboxDriver->currentText(); }
    QString getDataSourceName() { return pcomboboxDataSourceName->currentText(); }
    QString getUserID() { return plineeditUserID->text(); }
    QString getPassword() { return plineeditPassword->text(); }

protected:
    MYQTODBCEnvironment*      penvironment;
    QLabel *                plabelDriver;
    QComboBox *             pcomboboxDriver;
    QLabel *                plabelDataSourceName;
    QComboBox *             pcomboboxDataSourceName;
    QLabel *                plabelUserID;
	QLineEdit *             plineeditUserID;
    QLabel *                plabelPassword;
	QLineEdit *             plineeditPassword;
    MYQTODBCMessageOutput *   pmessageoutput;

    virtual void doLoadDrivers();
    virtual void doLoadDataSourceNames();
};

#endif


