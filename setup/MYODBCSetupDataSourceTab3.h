/* Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/ODBC is licensed under the terms of the
   GPL, like most MySQL Connectors. There are special exceptions
   to the terms and conditions of the GPL as it is applied to
   this software, see the FLOSS License Exception available on
   mysql.com.

   This program is free software; you can redistribute it and/or modify
   it under the terms of version 2 of the GNU General Public License as
   published by the Free Software Foundation.

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

#ifndef MYODBCUTIL_DATASOURCETAB3_H
#define MYODBCUTIL_DATASOURCETAB3_H

#include <qtabwidget.h>

#include "MYODBCSetupDataSourceTab3a.h"
#include "MYODBCSetupDataSourceTab3b.h"
#include "MYODBCSetupDataSourceTab3c.h"
#include "MYODBCSetupDataSourceTab3d.h"
#include "MYODBCSetupDataSourceTab3e.h"

class MYODBCSetupDataSourceDialog;

class MYODBCSetupDataSourceTab3 : public QWidget
{
    Q_OBJECT

    friend class MYODBCSetupDataSourceDialog;
public:
    MYODBCSetupDataSourceTab3( QWidget *pwidgetParent );

protected:
    QVBoxLayout *                playoutTop;
    QTabWidget *                 ptabwidget;
    MYODBCSetupDataSourceTab3a * ptab3connection;
    MYODBCSetupDataSourceTab3b * ptab3metadata;
    MYODBCSetupDataSourceTab3c * ptab3results;
    MYODBCSetupDataSourceTab3d * ptab3debug;
    MYODBCSetupDataSourceTab3e * ptab3misc;

    unsigned int  getFlags();
    bool          getInteractiveFlag();
};

#endif

