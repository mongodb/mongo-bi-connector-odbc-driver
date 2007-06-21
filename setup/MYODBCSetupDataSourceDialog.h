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

#ifndef MYODBCUTIL_DATASOURCEDIALOG_H
#define MYODBCUTIL_DATASOURCEDIALOG_H

#include <qapplication.h>
#include <qstring.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qmessagebox.h>
#include <qtextbrowser.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qprocess.h>
#include <qtooltip.h>

#include "MYODBCSetup.h"
#include "MYODBCSetupDataSourceTab1.h"
#include "MYODBCSetupDataSourceTab2.h"
#include "MYODBCSetupDataSourceTab3.h"
#include "MYODBCSetupAssistText.h"

class MYODBCSetupDataSourceDialog : public QDialog
{
    Q_OBJECT

public:
    MYODBCSetupDataSourceDialog( QWidget *pwidgetParent, MYODBCUTIL_DATASOURCE *pDataSource );
    MYODBCSetupDataSourceDialog( QWidget *pwidgetParent, SQLHDBC hDBC /* actually a (DBC*) */, MYODBCUTIL_DATASOURCE *pDataSource );
    ~MYODBCSetupDataSourceDialog();

public slots:
    void slotTest();
    void slotDiagnostics();
    void slotHelp();
    void slotOk();

protected:
    SQLHDBC                     hDBC;
    MYODBCUTIL_DATASOURCE *     pDataSource;

    // starts here
    QVBoxLayout *               playoutTop;
    QVBoxLayout *               playoutTop2; // so we can have zero padding around header and padding around rest

    // these are in playoutTop
    QHBoxLayout *               playoutLabels;
    QSplitter *                 psplitter;
    QHBoxLayout *               playoutButtons;
    QTextEdit *                 ptexteditDiagnostics;

    // these are in playoutLabels
    QLabel *                    plabelText;
    QLabel *                    plabelImage;

    // these are in psplitter
    QTabWidget *                ptabwidget;
    MYODBCSetupAssistText *     ptextbrowserAssist;

    // these are in the ptabwidget
    MYODBCSetupDataSourceTab1 * ptab1;
    MYODBCSetupDataSourceTab2 * ptab2;
    MYODBCSetupDataSourceTab3 * ptab3;

    // these are in playoutButtons
    QPushButton *               ppushbuttonTest;
    QPushButton *               ppushbuttonDiagnostics;
    QPushButton *               ppushbuttonHelp;
    QPushButton *               ppushbuttonOk;
    QPushButton *               ppushbuttonCancel;

    void doInit();
    void doApplyMode();

    BOOL doTestUsingDriverManager();
    BOOL doTestUsingDriver();
    BOOL doLoadCharsetNamesUsingDriverManager();
    BOOL doLoadCharsetNamesUsingDriver();
    BOOL doLoadDatabaseNamesUsingDriverManager();
    BOOL doLoadDatabaseNamesUsingDriver();

    QString buildConnectString();

protected slots:
    void slotShowDiagnostics( SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h );
    void slotShowInstallerError();
    void slotToggleGuru( bool b );
    void slotLoadDatabaseNames();
    void slotLoadCharsetNames();

};

#endif

