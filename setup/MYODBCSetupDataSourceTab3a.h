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

#ifndef MYODBCUTIL_DATASOURCETAB3A_H
#define MYODBCUTIL_DATASOURCETAB3A_H

#include <qobject.h>
#include <qstring.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtooltip.h>

#include "MYODBCSetupCheckBox.h"

class MYODBCSetupDataSourceDialog;
class MYODBCSetupDataSourceTab3;

class MYODBCSetupDataSourceTab3a : public QWidget
{
    Q_OBJECT

    friend class MYODBCSetupDataSourceDialog;
    friend class MYODBCSetupDataSourceTab3;
public:
    MYODBCSetupDataSourceTab3a( QWidget *pwidgetParent );

protected:

    MYODBCSetupCheckBox *pcheckboxAllowBigResults;
    MYODBCSetupCheckBox *pcheckboxUseCompressedProtocol;
    MYODBCSetupCheckBox *pcheckboxEnableReconnect;
    MYODBCSetupCheckBox *pcheckboxDontPromptOnConnect;
    MYODBCSetupCheckBox *pcheckboxForceUseOfNamedPipes;
    MYODBCSetupCheckBox *pcheckboxMultiStatements;
    MYODBCSetupCheckBox *pcheckboxInteractive;
};

#endif

