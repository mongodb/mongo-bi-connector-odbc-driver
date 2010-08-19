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


