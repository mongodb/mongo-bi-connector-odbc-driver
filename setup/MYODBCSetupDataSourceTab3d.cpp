/* Copyright (C) 2000-2005 MySQL AB

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

#include "MYODBCSetupDataSourceTab3d.h"

MYODBCSetupDataSourceTab3d::MYODBCSetupDataSourceTab3d( QWidget *pwidgetParent )
    : QWidget( pwidgetParent )
{
    QString         stringSaveQueries( tr("Enable query logging to `myodbc.sql' file. (Enabled only in debug mode.)") );
#if QT_VERSION >= 0x040000
    QVBoxLayout *   playoutFields = new QVBoxLayout;

    setLayout( playoutFields );
#else
    QVBoxLayout *   playoutFields = new QVBoxLayout( this );
#endif

    playoutFields->setMargin( 20 );
    playoutFields->setSpacing( 5 );
    playoutFields->addStretch( 10 );

    pcheckboxSaveQueries = new MYODBCSetupCheckBox( tr("Save Queries to myodbc.sql"), this );
    pcheckboxSaveQueries->setAssistText( stringSaveQueries );
    playoutFields->addWidget( pcheckboxSaveQueries );
#if QT_VERSION >= 0x040000
    pcheckboxSaveQueries->setToolTip( stringSaveQueries );
#else
    QToolTip::add( pcheckboxSaveQueries, stringSaveQueries );
#endif
    playoutFields->addStretch( 10 );
}


