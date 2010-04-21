/* Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYODBCUTIL_DATASOURCETAB3E_H
#define MYODBCUTIL_DATASOURCETAB3E_H

#include <qobject.h>
#include <qstring.h>
#include <qlayout.h>
#include <qtooltip.h>

#include "MYODBCSetupCheckBox.h"

class MYODBCSetupDataSourceDialog;
class MYODBCSetupDataSourceTab3;

class MYODBCSetupDataSourceTab3e : public QWidget
{
  Q_OBJECT

  friend class MYODBCSetupDataSourceDialog;
  friend class MYODBCSetupDataSourceTab3;
public:
  MYODBCSetupDataSourceTab3e( QWidget *pwidgetParent );

protected:
  MYODBCSetupCheckBox *pcheckboxSafe;
  MYODBCSetupCheckBox *pcheckboxDontUseSetLocale;
  MYODBCSetupCheckBox *pcheckboxIgnoreSpaceAfterFunctionNames;
  MYODBCSetupCheckBox *pcheckboxReadOptionsFromMyCnf;
  MYODBCSetupCheckBox *pcheckboxDisableTransactions;
  MYODBCSetupCheckBox *pcheckboxMinDate2Zero;
};

#endif

