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

#ifndef QTODBCPROMPT_H
#define QTODBCPROMPT_H

//
#include <qobject.h>
#include <qstringlist.h> 
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qpixmap.h>

class MYQTODBCPrompt : public QObject
{

    Q_OBJECT

public:

    enum PromptType 
	{ 
		PromptLabel, 
		PromptLineEdit, 
		PromptCombo, 
		PromptComboWrite, 
		PromptCheck 
	};

    MYQTODBCPrompt( PromptType nPromptType, const QString &stringName );
    virtual ~MYQTODBCPrompt();


    // SETTERS
    virtual void setName( const QString &stringName ) { this->stringName = stringName; }
    virtual void setHelp( const QString &stringHelp ) { this->stringHelp = stringHelp; }
    virtual void setOptions( const QString &stringOption ) { listOptions.append( stringOption ); }

    // GETTERS
    virtual QString getName() { return stringName; }
    virtual QString getValue() { return stringValue; }
    virtual QWidget *getEditor( QWidget * pwidgetParent );
    PromptType getPromptType() { return nPromptType; }

public slots:
    void setValue( int nValue ) { this->stringValue = QString::number( nValue ); }
    void setValue( const QString &stringValue );

protected:
    PromptType    nPromptType;

    // current state
    QString         stringName;
    QString         stringValue;

    QString         stringHelp;
    QStringList     listOptions;
};

#endif 


