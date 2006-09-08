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

#include "MYNodeODBCDataSources.h"

#include "DataSourcesUser16x16.xpm"
#include "DataSourcesSystem16x16.xpm"

MYNodeODBCDataSources::MYNodeODBCDataSources( QListView *pParent, SQLUSMALLINT nDataSourceType, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent )
{
    Init( nDataSourceType, penvironment );
}

MYNodeODBCDataSources::MYNodeODBCDataSources( QListViewItem *pParent, SQLUSMALLINT nDataSourceType, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent )
{
    Init( nDataSourceType, penvironment );
}

MYNodeODBCDataSources::MYNodeODBCDataSources( QListViewItem *pParent, QListViewItem *pAfter, SQLUSMALLINT nDataSourceType, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent, pAfter  )
{
    Init( nDataSourceType, penvironment );
}

MYNodeODBCDataSources::~MYNodeODBCDataSources()
{
    listDataSources.clear();
}

void MYNodeODBCDataSources::doLoadDataSources()
{
    SQLRETURN nReturn;
    SQLCHAR   szName[SQL_MAX_DSN_LENGTH+1];
    SQLCHAR   szDesc[SQL_MAX_DSN_LENGTH+1];

    nReturn = penvironment->getDataSource( nDataSourceType, szName, SQL_MAX_DSN_LENGTH, 0, szDesc, SQL_MAX_DSN_LENGTH, 0 );    
    while ( SQL_SUCCEEDED( nReturn ) )
    {
        listDataSources.append( new MYNodeODBCDataSource( this, nDataSourceType, (const char*)szName, (const char*)szDesc, penvironment ) );
        nReturn = penvironment->getDataSource( SQL_FETCH_NEXT, szName, SQL_MAX_DSN_LENGTH, 0, szDesc, SQL_MAX_DSN_LENGTH, 0 );    
    }
}

void MYNodeODBCDataSources::Init( SQLUSMALLINT nDataSourceType, MYQTODBCEnvironment * penvironment )
{
#ifdef Q_WS_MACX
    /*
     * iODBC does not currently support SQL_FETCH_FIRST_USER and SQL_FETCH_FIRST_SYSTEM
     * so return all data sources as if they are all User.
     */
    this->nDataSourceType = SQL_FETCH_FIRST;
    setText( 0, "User Data Sources" );
    setText( 1, "" );
    setText( 2, "" );
    setPixmap( 0, QPixmap( xpmDataSourcesUser16x16 ) );
#else
    switch ( nDataSourceType )
    {
    case SQL_FETCH_FIRST_SYSTEM:
        this->nDataSourceType = nDataSourceType;
        setText( 0, "System Data Sources" );
        setText( 1, "" );
        setText( 2, "" );
        setPixmap( 0, QPixmap( xpmDataSourcesSystem16x16 ) );
        break;
    default:
        this->nDataSourceType = SQL_FETCH_FIRST_USER;
        setText( 0, "User Data Sources" );
        setText( 1, "" );
        setText( 2, "" );
        setPixmap( 0, QPixmap( xpmDataSourcesUser16x16 ) );
    }
#endif

    this->penvironment = penvironment;
    listDataSources.setAutoDelete( TRUE );
    doLoadDataSources();
}

