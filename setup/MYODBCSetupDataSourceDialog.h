/* Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.

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

