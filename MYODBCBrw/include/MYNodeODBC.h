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

#ifndef MYNODEODBC_H
#define MYNODEODBC_H

// libqt
#include <qmessagebox.h>

// libQTODBC++
#include <MYQTODBCEnvironment.h>

#include "MYListView.h"
#include "MYNode.h"
#include "MYNodeODBCDrivers.h"
#include "MYNodeODBCDataSources.h"

class MYNodeODBC : public MYNode
{
    Q_OBJECT
public:
    MYNodeODBC( MYListView * pParent );
    MYNodeODBC( MYNode * pParent );
    ~MYNodeODBC();

    // SETTERS

    // GETTERS
    virtual MYQTODBCEnvironment *getEnvironment() { return penvironment; }

    // DOERS

    virtual void Menu( const QPoint &point );

signals:
    virtual void signalMessage( MYODBCMessage * );

public slots:
    virtual void slotProperties();
    virtual void slotLoadChildren();

private:
    MYQTODBCEnvironment *         penvironment;
    MYNodeODBCDrivers *        pDrivers;
    MYNodeODBCDataSources *    pDataSourcesUser;
    MYNodeODBCDataSources *    pDataSourcesSystem;

    void Init();
};

#endif

