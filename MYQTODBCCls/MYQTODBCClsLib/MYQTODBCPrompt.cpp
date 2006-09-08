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

#include "MYQTODBCPrompt.h"
    
MYQTODBCPrompt::MYQTODBCPrompt( PromptType nPromptType, const QString &stringName )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    this->nPromptType     = nPromptType;
    this->stringName      = stringName;

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCPrompt::~MYQTODBCPrompt()
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
QWidget * MYQTODBCPrompt::getEditor( QWidget * pwidgetParent )
{
    int n = getPromptType();

    if ( !pwidgetParent )
        return 0;

    switch ( n )
    {
        case PromptLabel:
        {
            QLabel *pwidget =  new QLabel( stringName, pwidgetParent );
            pwidget->setText( getValue() );
            return pwidget;
        }
        case PromptLineEdit:
        {
            QLineEdit *pwidget =  new QLineEdit( stringName, pwidgetParent );
            pwidget->setText( getValue() );
            connect( pwidget, SIGNAL(textChanged(const QString&)), SLOT(setValue(const QString&)) );
            return pwidget;
        }
        case PromptCombo:
        {
            QComboBox *pwidget =  new QComboBox( pwidgetParent );
            QString stringOption;
            int nItem = 0;

            for ( listOptions.begin(); listOptions.at( nItem ) != listOptions.end(); nItem++ ) 
            {
                stringOption = (* (listOptions.at(nItem)) );
                pwidget->insertItem( stringOption );

                if ( stringOption == getValue() )
                    pwidget->setCurrentItem( nItem );
            }
            connect( pwidget, SIGNAL(activated(const QString&)), SLOT(setValue(const QString&)) );
            return pwidget;
        }
        case PromptComboWrite:
        {
            QComboBox *pwidget =  new QComboBox( true, pwidgetParent );
            QString string;
            int nItem = 0;

            for ( listOptions.begin(); listOptions.at( nItem ) != listOptions.end(); nItem++ ) 
            {
                string = (* (listOptions.at(nItem)) );
                pwidget->insertItem( string );
                if ( string == getValue() )
                    pwidget->setCurrentItem( nItem );
            }
            connect( pwidget, SIGNAL(activated(const QString&)), SLOT(setValue(const QString&)) );
            return pwidget;
        }
        case PromptCheck:
        {
            QCheckBox *pwidget =  new QCheckBox( pwidgetParent );
            pwidget->setChecked( getValue().toInt() ); 
            connect( pwidget, SIGNAL(stateChanged(int)), SLOT(setValue(int)) );
            return pwidget;
        }
    }

    return 0;
}

/*!

*/
void MYQTODBCPrompt::setValue( const QString &stringValue ) 
{ 
    this->stringValue = stringValue; 

//    uint n0 = 0;
//    uint n1 = listOptions.findIndex( stringValue );
}


