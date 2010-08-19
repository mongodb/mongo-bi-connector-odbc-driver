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

#include "MYODBCSetupDataSourceTab3b.h"

MYODBCSetupDataSourceTab3b::MYODBCSetupDataSourceTab3b( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    QString         stringChangeBIGINTColumnsToInt( tr("Change LONGLONG columns to INT columns (some applications can't handle LONGLONG).") );
    QString         stringDisableBinaryResult( tr("Always handle binary function results as character data") );
    QString         stringIgnorePoundInTable( tr("Ignore use of database name in db_name.tbl_name.col_name.") );
    QString         stringReturnTableNamesSQLDescribeCol( tr("SQLDescribeCol() will return fully qualified column names.") );
    QString         stringNoCatalog( tr("Return 'user' as Table_qualifier and Table_owner from SQLTables (experimental).") );
    QString         stringCapColumnSize( tr("Limit reported column size to signed 32-bit integer (possible workaround for some applications, automatically enabled for applications using ADO)") );
    QString         stringNoI_S( tr("Do not use INFORMATION_SCHEMA for catalog data") );

#if QT_VERSION >= 0x040000
    QVBoxLayout *   playoutFields = new QVBoxLayout();
    setLayout( playoutFields );
#else
    QVBoxLayout *   playoutFields = new QVBoxLayout( this );
#endif

    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxChangeBIGINTColumnsToInt = new MYODBCSetupCheckBox( tr("Change BIGINT Columns To Int"), this );
    pcheckboxChangeBIGINTColumnsToInt->setAssistText( stringChangeBIGINTColumnsToInt );
    playoutFields->addWidget( pcheckboxChangeBIGINTColumnsToInt );
#if QT_VERSION >= 0x040000
    pcheckboxChangeBIGINTColumnsToInt->setToolTip( stringChangeBIGINTColumnsToInt );
#else
    QToolTip::add( pcheckboxChangeBIGINTColumnsToInt, stringChangeBIGINTColumnsToInt );
#endif

    pcheckboxDisableBinaryResult = new MYODBCSetupCheckBox( tr("Always handle binary function results as character data"), this );
    pcheckboxDisableBinaryResult->setAssistText( stringDisableBinaryResult );
    playoutFields->addWidget( pcheckboxDisableBinaryResult );
#if QT_VERSION >= 0x040000
    pcheckboxDisableBinaryResult->setToolTip( stringDisableBinaryResult );
#else
    QToolTip::add( pcheckboxDisableBinaryResult, stringDisableBinaryResult );
#endif

    pcheckboxIgnorePoundInTable = new MYODBCSetupCheckBox( tr("Ignore # In Table Name"), this );
    pcheckboxIgnorePoundInTable->setAssistText( stringIgnorePoundInTable );
    playoutFields->addWidget( pcheckboxIgnorePoundInTable );
#if QT_VERSION >= 0x040000
    pcheckboxIgnorePoundInTable->setToolTip( stringIgnorePoundInTable );
#else
    QToolTip::add( pcheckboxIgnorePoundInTable, stringIgnorePoundInTable );
#endif

    pcheckboxReturnTableNamesSQLDescribeCol = new MYODBCSetupCheckBox( tr("Return Table Names For SQLDescribeCol"), this );
    pcheckboxReturnTableNamesSQLDescribeCol->setAssistText( stringReturnTableNamesSQLDescribeCol );
    playoutFields->addWidget( pcheckboxReturnTableNamesSQLDescribeCol );
#if QT_VERSION >= 0x040000
    pcheckboxReturnTableNamesSQLDescribeCol->setToolTip( stringReturnTableNamesSQLDescribeCol );
#else
    QToolTip::add( pcheckboxReturnTableNamesSQLDescribeCol, stringReturnTableNamesSQLDescribeCol );
#endif

    pcheckboxNoCatalog = new MYODBCSetupCheckBox( tr("No Catalog (exp)"), this );
    pcheckboxNoCatalog->setAssistText( stringNoCatalog );
    playoutFields->addWidget( pcheckboxNoCatalog );
#if QT_VERSION >= 0x040000
    pcheckboxNoCatalog->setToolTip( stringNoCatalog );
#else
    QToolTip::add( pcheckboxNoCatalog, stringNoCatalog );
#endif

    pcheckboxCapColumnSize = new MYODBCSetupCheckBox( tr("Limit column size to signed 32-bit range"), this );
    pcheckboxCapColumnSize->setAssistText( stringCapColumnSize );
    playoutFields->addWidget( pcheckboxCapColumnSize );
#if QT_VERSION >= 0x040000
    pcheckboxCapColumnSize->setToolTip( stringCapColumnSize );
#else
    QToolTip::add( pcheckboxCapColumnSize, stringCapColumnSize );
#endif

    pcheckboxNoI_S = new MYODBCSetupCheckBox( tr("Do not use INFORMATION_SCHEMA for metadata"), this );
    pcheckboxNoI_S->setAssistText( stringNoI_S );
    playoutFields->addWidget( pcheckboxNoI_S );
    MYODBC_ADD_TOOLTIP(pcheckboxNoI_S, stringNoI_S);

    playoutFields->addStretch( 10 );
}


