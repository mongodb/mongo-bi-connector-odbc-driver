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

#include "MYODBCSetupDataSourceTab3.h"

MYODBCSetupDataSourceTab3::MYODBCSetupDataSourceTab3( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
#if QT_VERSION >= 0x040000
    QVBoxLayout *playoutFields = new QVBoxLayout;
    setLayout( playoutFields );
#else
    QVBoxLayout *playoutFields = new QVBoxLayout( this );
#endif
    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );

    ptabwidget = new QTabWidget( this );
    playoutFields->addWidget( ptabwidget );

    ptab3connection = new MYODBCSetupDataSourceTab3a( ptabwidget );
    ptabwidget->addTab( ptab3connection, tr("Connection") ); 
    ptab3metadata = new MYODBCSetupDataSourceTab3b( ptabwidget );
    ptabwidget->addTab( ptab3metadata, tr("Metadata") ); 
    ptab3results = new MYODBCSetupDataSourceTab3c( ptabwidget );
    ptabwidget->addTab( ptab3results, tr("Cursor/Result") ); 
    ptab3debug = new MYODBCSetupDataSourceTab3d( ptabwidget );
    ptabwidget->addTab( ptab3debug, tr("Dbg") );
    ptab3misc = new MYODBCSetupDataSourceTab3e( ptabwidget );
    ptabwidget->addTab( ptab3misc, tr("Misc") );
}

unsigned int MYODBCSetupDataSourceTab3::getFlags()
{
    unsigned int nFlags = 0;

    if ( ptab3results->pcheckboxReturnMatchingRows->isChecked() )
        nFlags |= 1 << 1;
    if ( ptab3connection->pcheckboxAllowBigResults->isChecked() )
        nFlags |= 1 << 3;
    if ( ptab3connection->pcheckboxDontPromptOnConnect->isChecked() )
        nFlags |= 1 << 4;
    if ( ptab3results->pcheckboxEnableDynamicCursor->isChecked() )
        nFlags |= 1 << 5;
    if ( ptab3metadata->pcheckboxIgnorePoundInTable->isChecked() )
        nFlags |= 1 << 6;
    if ( ptab3results->pcheckboxUseManagerCursors->isChecked() )
        nFlags |= 1 << 7;
    if ( ptab3misc->pcheckboxDontUseSetLocale->isChecked() )
        nFlags |= 1 << 8;
    if ( ptab3results->pcheckboxPadCharToFullLen->isChecked() )
        nFlags |= 1 << 9;
    if ( ptab3metadata->pcheckboxReturnTableNamesSQLDescribeCol->isChecked() )
        nFlags |= 1 << 10;
    if ( ptab3connection->pcheckboxUseCompressedProtocol->isChecked() )
        nFlags |= 1 << 11;
    if ( ptab3misc->pcheckboxIgnoreSpaceAfterFunctionNames->isChecked() ) 
        nFlags |= 1 << 12;
    if ( ptab3connection->pcheckboxForceUseOfNamedPipes->isChecked() )          
        nFlags |= 1 << 13;
    if ( ptab3metadata->pcheckboxChangeBIGINTColumnsToInt->isChecked() )
        nFlags |= 1 << 14;
    if ( ptab3metadata->pcheckboxNoCatalog->isChecked() )
        nFlags |= 1 << 15;
    if ( ptab3misc->pcheckboxReadOptionsFromMyCnf->isChecked() )          
        nFlags |= 1 << 16;
    if ( ptab3misc->pcheckboxSafe->isChecked() )
        nFlags |= 1 << 17;
    if ( ptab3misc->pcheckboxDisableTransactions->isChecked() )           
        nFlags |= 1 << 18;
    if ( ptab3debug->pcheckboxSaveQueries->isChecked() )
        nFlags |= 1 << 19;
    if ( ptab3results->pcheckboxDontCacheResults->isChecked() )
        nFlags |= 1 << 20;
    if ( ptab3results->pcheckboxForceUseOfForwardOnlyCursors->isChecked() )  
        nFlags |= 1 << 21;
    if ( ptab3connection->pcheckboxEnableReconnect->isChecked() )
        nFlags |= 1 << 22;
    if ( ptab3results->pcheckboxAutoIncrementIsNull->isChecked() )
        nFlags |= 1 << 23;
    if ( ptab3results->pcheckboxZeroDate2Min->isChecked() )
        nFlags |= 1 << 24;
    if ( ptab3misc->pcheckboxMinDate2Zero->isChecked() )
      nFlags |= 1 << 25;
    if ( ptab3connection->pcheckboxMultiStatements->isChecked() )
        nFlags |= 1 << 26;
    if ( ptab3metadata->pcheckboxCapColumnSize->isChecked() )
        nFlags |= 1 << 27;
    if ( ptab3metadata->pcheckboxDisableBinaryResult->isChecked() )
        nFlags |= 1 << 28;
    if ( ptab3metadata->pcheckboxNoI_S->isChecked() )
        nFlags |= 1 << 30;

    return nFlags;
}


bool MYODBCSetupDataSourceTab3::getInteractiveFlag()
{
  return ptab3connection->pcheckboxInteractive->isChecked();
}
