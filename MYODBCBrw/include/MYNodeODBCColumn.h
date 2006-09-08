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

#ifndef MYNODEODBCCOLUMN_H
#define MYNODEODBCCOLUMN_H

#include <qstring.h>
#include <qmessagebox.h>
#include <qpixmap.h>

#include <MYQTODBCConnection.h>

#include "MYNode.h"

class MYNodeODBCColumn : public MYNode
{
    Q_OBJECT
public:
    MYNodeODBCColumn( QListView         *pParent, MYQTODBCConnection *pconnection = 0, char *pszName = 0, char *pszType = 0, char *pszDesc = 0 );
    MYNodeODBCColumn( QListViewItem     *pParent, MYQTODBCConnection *pconnection = 0, char *pszName = 0, char *pszType = 0, char *pszDesc = 0 );
    MYNodeODBCColumn( QListViewItem     *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection = 0, char *pszName = 0, char *pszType = 0, char *pszDesc = 0 );
    ~MYNodeODBCColumn();

    void setOpen( bool );
    void setup();
    void selectionChanged( QListViewItem * );

private:
    MYQTODBCConnection *  pconnection;
    QString    qsName;

    void Init( MYQTODBCConnection *pconnection, char *pszName, char *pszType, char *pszDesc );
};

#endif

