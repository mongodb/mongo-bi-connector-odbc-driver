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

#include "MYODBCSetupDataSourceTab3c.h"

MYODBCSetupDataSourceTab3c::MYODBCSetupDataSourceTab3c( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    QString         stringReturnTableNamesSQLDescribeCol( tr("SQLDescribeCol() will return fully qualified column names.") );
    QString         stringIgnoreSpaceAfterFunctionNames( tr("Tell server to ignore space after function name and before `(' (needed by PowerBuilder). This will make all function names keywords.") );
    QString         stringForceUseOfNamedPipes( tr("Connect with named pipes to a mysqld server running on NT.") );
    QString         stringNoCatalog( tr("Return 'user' as Table_qualifier and Table_owner from SQLTables (experimental).") );
    QString         stringReadOptionsFromMyCnf( tr("Read parameters from the [client] and [odbc] groups from `my.cnf'.") );
    QString         stringDisableTransactions( tr("Disable transactions.") );
    QString         stringForceUseOfForwardOnlyCursors( tr("Force the use of Forward-only cursor type. In case of applications setting the default static/dynamic cursor type, and one wants driver to use non-cache result sets, then this option will ensure the forward-only cursor behavior.") );
    QString         stringMultiStatements( tr("Allow multiple statements in a single query.") );
    QString         stringCapColumnSize( tr("Limit reported column size to signed 32-bit integer (possible workaround for some applications, automatically enabled for applications using ADO)") );

#if QT_VERSION >= 0x040000
    QVBoxLayout *playoutFields = new QVBoxLayout;
    setLayout( playoutFields );
#else
    QVBoxLayout *playoutFields = new QVBoxLayout( this );
#endif

    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxReturnTableNamesSQLDescribeCol = new MYODBCSetupCheckBox( tr("Return Table Names For SQLDescribeCol"), this );
    pcheckboxReturnTableNamesSQLDescribeCol->setAssistText( stringReturnTableNamesSQLDescribeCol );
    playoutFields->addWidget( pcheckboxReturnTableNamesSQLDescribeCol );
#if QT_VERSION >= 0x040000
    pcheckboxReturnTableNamesSQLDescribeCol->setToolTip( stringReturnTableNamesSQLDescribeCol );
#else
    QToolTip::add( pcheckboxReturnTableNamesSQLDescribeCol, stringReturnTableNamesSQLDescribeCol );
#endif

    pcheckboxIgnoreSpaceAfterFunctionNames = new MYODBCSetupCheckBox( tr("Ignore Space After Function Names"), this );
    pcheckboxIgnoreSpaceAfterFunctionNames->setAssistText( stringIgnoreSpaceAfterFunctionNames );
    playoutFields->addWidget( pcheckboxIgnoreSpaceAfterFunctionNames );
#if QT_VERSION >= 0x040000
    pcheckboxIgnoreSpaceAfterFunctionNames->setToolTip( stringIgnoreSpaceAfterFunctionNames );
#else
    QToolTip::add( pcheckboxIgnoreSpaceAfterFunctionNames, stringIgnoreSpaceAfterFunctionNames );
#endif

    pcheckboxForceUseOfNamedPipes = new MYODBCSetupCheckBox( tr("Force Use Of Named Pipes"), this );
    pcheckboxForceUseOfNamedPipes->setAssistText( stringForceUseOfNamedPipes );
    playoutFields->addWidget( pcheckboxForceUseOfNamedPipes );
#if QT_VERSION >= 0x040000
    pcheckboxForceUseOfNamedPipes->setToolTip( stringForceUseOfNamedPipes );
#else
    QToolTip::add( pcheckboxForceUseOfNamedPipes, stringForceUseOfNamedPipes );
#endif

    pcheckboxNoCatalog = new MYODBCSetupCheckBox( tr("No Catalog (exp)"), this );
    pcheckboxNoCatalog->setAssistText( stringNoCatalog );
    playoutFields->addWidget( pcheckboxNoCatalog );
#if QT_VERSION >= 0x040000
    pcheckboxNoCatalog->setToolTip( stringNoCatalog );
#else
    QToolTip::add( pcheckboxNoCatalog, stringNoCatalog );
#endif

    pcheckboxReadOptionsFromMyCnf = new MYODBCSetupCheckBox( tr("Read Options From my.cnf"), this );
    pcheckboxReadOptionsFromMyCnf->setAssistText( stringReadOptionsFromMyCnf );
    playoutFields->addWidget( pcheckboxReadOptionsFromMyCnf );
#if QT_VERSION >= 0x040000
    pcheckboxReadOptionsFromMyCnf->setToolTip( stringReadOptionsFromMyCnf );
#else
    QToolTip::add( pcheckboxReadOptionsFromMyCnf, stringReadOptionsFromMyCnf );
#endif

    pcheckboxDisableTransactions = new MYODBCSetupCheckBox( tr("Disable Transactions"), this );
    pcheckboxDisableTransactions->setAssistText( stringDisableTransactions );
    playoutFields->addWidget( pcheckboxDisableTransactions );
#if QT_VERSION >= 0x040000
    pcheckboxDisableTransactions->setToolTip( stringDisableTransactions );
#else
    QToolTip::add( pcheckboxDisableTransactions, stringDisableTransactions );
#endif

    pcheckboxForceUseOfForwardOnlyCursors = new MYODBCSetupCheckBox( tr("Force Use Of Forward Only Cursors"), this );
    pcheckboxForceUseOfForwardOnlyCursors->setAssistText( stringForceUseOfForwardOnlyCursors );
    playoutFields->addWidget( pcheckboxForceUseOfForwardOnlyCursors );
#if QT_VERSION >= 0x040000
    pcheckboxForceUseOfForwardOnlyCursors->setToolTip( stringForceUseOfForwardOnlyCursors );
#else
    QToolTip::add( pcheckboxForceUseOfForwardOnlyCursors, stringForceUseOfForwardOnlyCursors );
#endif

    pcheckboxMultiStatements = new MYODBCSetupCheckBox( tr("Allow multiple statements"), this );
    pcheckboxMultiStatements->setAssistText( stringMultiStatements );
    playoutFields->addWidget( pcheckboxMultiStatements );
#if QT_VERSION >= 0x040000
    pcheckboxMultiStatements->setToolTip( stringMultiStatements );
#else
    QToolTip::add( pcheckboxMultiStatements, stringMultiStatements );
#endif

    pcheckboxCapColumnSize = new MYODBCSetupCheckBox( tr("Limit column size to signed 32-bit range"), this );
    pcheckboxCapColumnSize->setAssistText( stringCapColumnSize );
    playoutFields->addWidget( pcheckboxCapColumnSize );
#if QT_VERSION >= 0x040000
    pcheckboxCapColumnSize->setToolTip( stringCapColumnSize );
#else
    QToolTip::add( pcheckboxCapColumnSize, stringCapColumnSize );
#endif

    playoutFields->addStretch( 10 );
}


