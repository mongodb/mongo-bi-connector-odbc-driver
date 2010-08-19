/* Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.

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

#include "MYODBCSetupDataSourceTab3e.h"

MYODBCSetupDataSourceTab3e::MYODBCSetupDataSourceTab3e( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
  QString       stringSafe( tr("Add some extra safety checks (should not be needed but...).") );
  QString       stringDontUseSetLocale( tr("Disable the use of extended fetch (experimental).") );
  QString       stringIgnoreSpaceAfterFunctionNames( tr("Tell server to ignore space after function name and before `(' (needed by PowerBuilder). This will make all function names keywords.") );
  QString       stringReadOptionsFromMyCnf( tr("Read parameters from the [client] and [odbc] groups from `my.cnf'.") );
  QString       stringDisableTransactions( tr("Disable transactions.") );
  QString       stringMinDate2Zero( tr("Bind minimal date as zero date.") );

#if QT_VERSION >= 0x040000
  QVBoxLayout * playoutFields= new QVBoxLayout;

  setLayout( playoutFields );
#else
  QVBoxLayout * playoutFields= new QVBoxLayout(this);
#endif

  playoutFields->setMargin(20);
  playoutFields->setSpacing(5);
  playoutFields->addStretch(10);

  pcheckboxSafe = new MYODBCSetupCheckBox(tr("Safe"), this);
  pcheckboxSafe->setAssistText(stringSafe);
  playoutFields->addWidget(pcheckboxSafe);
  MYODBC_ADD_TOOLTIP(pcheckboxSafe, stringSafe);

  pcheckboxDontUseSetLocale = new MYODBCSetupCheckBox( tr("Don't Use Set Locale"), this);
  pcheckboxDontUseSetLocale->setAssistText(stringDontUseSetLocale);
  playoutFields->addWidget(pcheckboxDontUseSetLocale );
  MYODBC_ADD_TOOLTIP(pcheckboxDontUseSetLocale, stringDontUseSetLocale);

  pcheckboxIgnoreSpaceAfterFunctionNames = new MYODBCSetupCheckBox(tr("Ignore Space After Function Names"), this);
  pcheckboxIgnoreSpaceAfterFunctionNames->setAssistText( stringIgnoreSpaceAfterFunctionNames);
  playoutFields->addWidget( pcheckboxIgnoreSpaceAfterFunctionNames );
  MYODBC_ADD_TOOLTIP(pcheckboxIgnoreSpaceAfterFunctionNames, stringIgnoreSpaceAfterFunctionNames);

  pcheckboxReadOptionsFromMyCnf = new MYODBCSetupCheckBox( tr("Read Options From my.cnf"), this );
  pcheckboxReadOptionsFromMyCnf->setAssistText( stringReadOptionsFromMyCnf );
  playoutFields->addWidget( pcheckboxReadOptionsFromMyCnf );
  MYODBC_ADD_TOOLTIP(pcheckboxReadOptionsFromMyCnf, stringReadOptionsFromMyCnf);

  pcheckboxDisableTransactions = new MYODBCSetupCheckBox( tr("Disable Transactions"), this );
  pcheckboxDisableTransactions->setAssistText( stringDisableTransactions );
  playoutFields->addWidget( pcheckboxDisableTransactions );
  MYODBC_ADD_TOOLTIP(pcheckboxDisableTransactions, stringDisableTransactions);

  pcheckboxMinDate2Zero = new MYODBCSetupCheckBox( tr("Bind minimal date as zero date"), this );
  pcheckboxMinDate2Zero->setAssistText( stringMinDate2Zero );
  playoutFields->addWidget( pcheckboxMinDate2Zero );
  MYODBC_ADD_TOOLTIP(pcheckboxMinDate2Zero, stringMinDate2Zero);

  playoutFields->addStretch( 10 );
}


