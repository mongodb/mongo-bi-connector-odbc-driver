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

#ifndef MYNODEODBCTABLETYPE_H
#define MYNODEODBCTABLETYPE_H

//
#include <stdio.h>

//
#include <qstring.h>
#include <qmessagebox.h>
#include <qptrlist.h>

//
#include <MYQTODBCConnection.h>
#include <MYQTODBCStatement.h>

//
#include "MYNode.h"
#include "MYNodeODBCTable.h"
#include "MYNodeODBCExtendedInfoItem.h"

class MYNodeODBCTableType : public MYNode
{
    Q_OBJECT
public:
    MYNodeODBCTableType( QListView         *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType );
    MYNodeODBCTableType( QListViewItem     *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType );
    MYNodeODBCTableType( QListViewItem     *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType );
    ~MYNodeODBCTableType();

    virtual void setOpen( bool bOpen );
    virtual void selectionChanged( QListViewItem * );
    virtual void setup();

protected:
    QPtrList<MYNode>           listChildren;
    MYQTODBCConnection *          pconnection;
    QString                     stringCatalog;
    QString                     stringSchema;
    QString                     stringType;

    void Init( MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType );
    void doLoadChildren();
};

#endif

