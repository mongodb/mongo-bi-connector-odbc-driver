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

    ptab3a = new MYODBCSetupDataSourceTab3a( ptabwidget );
    ptabwidget->addTab( ptab3a, tr("Flags 1") ); 
    ptab3b = new MYODBCSetupDataSourceTab3b( ptabwidget );
    ptabwidget->addTab( ptab3b, tr("Flags 2") ); 
    ptab3c = new MYODBCSetupDataSourceTab3c( ptabwidget );
    ptabwidget->addTab( ptab3c, tr("Flags 3") ); 
    ptab3d = new MYODBCSetupDataSourceTab3d( ptabwidget );
    ptabwidget->addTab( ptab3d, tr("Debug") ); 

}

unsigned int MYODBCSetupDataSourceTab3::getFlags()
{
    unsigned int nFlags = 0;

    if ( ptab3a->pcheckboxReturnMatchingRows->isChecked() )
        nFlags |= 1 << 1;
    if ( ptab3a->pcheckboxAllowBigResults->isChecked() )
        nFlags |= 1 << 3;
    if ( ptab3b->pcheckboxDontPromptOnConnect->isChecked() )
        nFlags |= 1 << 4;
    if ( ptab3b->pcheckboxEnableDynamicCursor->isChecked() )
        nFlags |= 1 << 5;
    if ( ptab3b->pcheckboxIgnorePoundInTable->isChecked() )
        nFlags |= 1 << 6;
    if ( ptab3b->pcheckboxUseManagerCursors->isChecked() )
        nFlags |= 1 << 7;
    if ( ptab3b->pcheckboxDontUseSetLocale->isChecked() )
        nFlags |= 1 << 8;
    if ( ptab3b->pcheckboxPadCharToFullLen->isChecked() )
        nFlags |= 1 << 9;
    if ( ptab3c->pcheckboxReturnTableNamesSQLDescribeCol->isChecked() )
        nFlags |= 1 << 10;
    if ( ptab3a->pcheckboxUseCompressedProtocol->isChecked() )
        nFlags |= 1 << 11;
    if ( ptab3c->pcheckboxIgnoreSpaceAfterFunctionNames->isChecked() ) 
        nFlags |= 1 << 12;
    if ( ptab3c->pcheckboxForceUseOfNamedPipes->isChecked() )          
        nFlags |= 1 << 13;
    if ( ptab3a->pcheckboxChangeBIGINTColumnsToInt->isChecked() )
        nFlags |= 1 << 14;
    if ( ptab3c->pcheckboxNoCatalog->isChecked() )
        nFlags |= 1 << 15;
    if ( ptab3c->pcheckboxReadOptionsFromMyCnf->isChecked() )          
        nFlags |= 1 << 16;
    if ( ptab3a->pcheckboxSafe->isChecked() )
        nFlags |= 1 << 17;
    if ( ptab3c->pcheckboxDisableTransactions->isChecked() )           
        nFlags |= 1 << 18;
    if ( ptab3d->pcheckboxSaveQueries->isChecked() )
        nFlags |= 1 << 19;
    if ( ptab3b->pcheckboxDontCacheResults->isChecked() )
        nFlags |= 1 << 20;
    if ( ptab3c->pcheckboxForceUseOfForwardOnlyCursors->isChecked() )  
        nFlags |= 1 << 21;
    if ( ptab3a->pcheckboxEnableReconnect->isChecked() )
        nFlags |= 1 << 22;
    if ( ptab3a->pcheckboxAutoIncrementIsNull->isChecked() )
        nFlags |= 1 << 23;
    if ( ptab3c->pcheckboxMultiStatements->isChecked() )
        nFlags |= 1 << 26;
    if ( ptab3c->pcheckboxCapColumnSize->isChecked() )
        nFlags |= 1 << 27;

    return nFlags;
}

