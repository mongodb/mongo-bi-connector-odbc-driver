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

#include "MYNodeODBCDrivers.h"

#include "Drivers16x16.xpm"
// #include "Driver.xpm"

MYNodeODBCDrivers::MYNodeODBCDrivers( QListView *pParent, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent )
{
    Init( penvironment );
}

MYNodeODBCDrivers::MYNodeODBCDrivers( QListViewItem *pParent, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent )
{
    Init( penvironment );
}

MYNodeODBCDrivers::MYNodeODBCDrivers( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCEnvironment * penvironment )
    : MYNode( pParent, pAfter  )
{
    Init( penvironment );
}

MYNodeODBCDrivers::~MYNodeODBCDrivers()
{
    listDrivers.clear();
}

void MYNodeODBCDrivers::doLoadDrivers()
{
    SQLRETURN   nReturn;
    SQLCHAR     szName[100];
    SQLSMALLINT nLengthNameReturned = 0;
    SQLCHAR     szAttributes[1024];
    SQLSMALLINT nLengthAttributesReturned = 0;

    *szName = '\0';
    nReturn = penvironment->getDriver( SQL_FETCH_FIRST, szName, sizeof(szName) - 1, &nLengthNameReturned, szAttributes, sizeof(szAttributes) - 1, &nLengthAttributesReturned );    
    while (  SQL_SUCCEEDED( nReturn ) && *szName ) // checking szName is a work-around for iODBC problem where SQLDrivers() is not implemented in iODBC at this time but it return success anyway
    {
        listDrivers.append( new MYNodeODBCDriver( this, (const char*)szName, QString::null, penvironment ) );
        *szName = '\0';
        nReturn = penvironment->getDriver( SQL_FETCH_NEXT, szName, sizeof(szName) - 1, &nLengthNameReturned, szAttributes, sizeof(szAttributes) - 1, &nLengthAttributesReturned );    
    }
}

void MYNodeODBCDrivers::Init( MYQTODBCEnvironment *penvironment )
{
    this->penvironment = penvironment;
    listDrivers.setAutoDelete( TRUE );
    setText( 0, "Drivers" );
    setText( 1, "" );
    setText( 2, "" );
    setPixmap( 0, QPixmap( xpmDrivers16x16 ) );

    doLoadDrivers();
}

