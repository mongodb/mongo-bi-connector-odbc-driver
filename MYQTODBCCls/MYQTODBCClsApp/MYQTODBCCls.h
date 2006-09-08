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

#ifndef QTODBC_H
#define QTODBC_H

// libqt
#include <qmainwindow.h>
#include <qstring.h>
#include <qaction.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtextedit.h>
#include <qtable.h>
#include <qsplitter.h>

// libODBC++
#include <MYODBCMessage.h>

// libQTODBC++
#include <MYQTODBCEnvironment.h>
#include <MYQTODBCConnection.h>
#include <MYQTODBCStatement.h>

class qtodbc : public QMainWindow
{ 
    Q_OBJECT
public:
	qtodbc();
	~qtodbc();

    // SETTERS

    // GETTERS

    // DO'rs

protected:
    MYQTODBCEnvironment * penvironment;
    MYQTODBCConnection *  pconnection;
    MYQTODBCStatement *   pstatement;

    QToolBar *          ptoolbar;
    QAction *           pactionConnect;
    QAction *           pactionExecute;

    QSplitter *         psplitter;
    QTextEdit *         ptexteditSQL;
    QTable *            ptableResults;
    QTextEdit *         ptexteditMessages;

    // SETTERS

    // GETTERS

    // DO'rs
    virtual void doResultGUIGrid();
    virtual void doResultGUIGridHeader( SWORD nColumns );
    virtual void doResultGUIGridBody( SWORD nColumns );

    // EVENTS

protected slots:
    virtual void slotConnectToggle();
    virtual void slotConnected();
    virtual void slotDisconnected();
    virtual void slotExecute();
    virtual void slotResults( MYQTODBCStatement *pstatement );
    virtual void slotMessage( MYODBCMessage *pmessage );
};

#endif


