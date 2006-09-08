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

#include "MYQTODBCPromptTableItem.h"
    
MYQTODBCPromptTableItem::MYQTODBCPromptTableItem( QTable * ptable, EditType nEditType, MYQTODBCPrompt * pprompt )
    : QTableItem( ptable, nEditType, pprompt->getValue() )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    this->pprompt = pprompt;

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCPromptTableItem::~MYQTODBCPromptTableItem()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif


#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!

*/
QWidget * MYQTODBCPromptTableItem::createEditor() const
{
    return pprompt->getEditor( table()->viewport() );
}

/*!

*/
void MYQTODBCPromptTableItem::setContentFromEditor( QWidget * )
{
    setText( pprompt->getValue() );
}


