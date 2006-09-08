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

#include "MYNodeODBCCatalog.h"

#include "CabinetClosed16x16.xpm"

MYNodeODBCCatalog::MYNodeODBCCatalog( QListView *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog );
}

MYNodeODBCCatalog::MYNodeODBCCatalog( QListViewItem *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog );
}

MYNodeODBCCatalog::MYNodeODBCCatalog( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString &stringCatalog )
    : MYNode( pParent, pAfter  )
{
    Init( pconnection, stringCatalog );
}

MYNodeODBCCatalog::~MYNodeODBCCatalog()
{
    Fini();
}

void MYNodeODBCCatalog::Init( MYQTODBCConnection *pconnection, const QString &stringCatalog )
{
    this->pconnection   = pconnection;
    this->stringCatalog    = stringCatalog;
    setText( 0, stringCatalog );
    setText( 1, "CATALOG (Data Dictionary)" );
    sortChildItems( 1, FALSE );
    listSchemas.setAutoDelete( TRUE );
    setPixmap( 0, QPixmap( xpmCabinetClosed16x16 ) );
}

void MYNodeODBCCatalog::Fini()
{
    listSchemas.clear();
}

void MYNodeODBCCatalog::setOpen( bool bOpen )
{
    QListViewItem::setOpen( bOpen );
    listView()->setSelected( listView()->selectedItem(), false );
    if ( bOpen )
    {
        doLoadSchemas();
        // If have less than 1 children than we know that we failed to find any schemas
        // so create a dummy schema.
        if ( listSchemas.count() < 1 )
        {
            listSchemas.append( new MYNodeODBCSchema( this, NULL, pconnection, stringCatalog, "" ) );
        }
    }
    else
    {
        Fini();
    }
}

void MYNodeODBCCatalog::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}

/*!
    doLoadSchemas
    
*/    
void MYNodeODBCCatalog::doLoadSchemas()
{
    MYQTODBCStatement *   pstatement  = new MYQTODBCStatement( pconnection );
    SQLRETURN            nReturn;
    SQLCHAR             szSchema[101];
    SQLINTEGER          nSchema;
    MYNodeODBCSchema*  pschema     = NULL;

    // SQL_ALL_SCHEMAS only works on v3 drivers but when used on a v2 driver this 
    // should simply fail to return a result with rows and that should be ok for us.
    if ( !SQL_SUCCEEDED( pstatement->getSchemas( stringCatalog.latin1(), SQL_ALL_SCHEMAS ) ) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        *szSchema = '\0';

        nReturn = pstatement->getData( 2, SQL_C_CHAR, szSchema, sizeof(szSchema) - 1, &nSchema );
        if ( SQL_SUCCEEDED( nReturn ) && nSchema != SQL_NULL_DATA && *szSchema != '\0' )
            listSchemas.append( pschema = new MYNodeODBCSchema( this, pschema, pconnection, stringCatalog, (const char *)szSchema ) );
    }

    delete pstatement;
}

void MYNodeODBCCatalog::selectionChanged( QListViewItem *p )
{
    MYNodeODBCSchema * pschema;

    for ( pschema = listSchemas.first(); pschema != 0; pschema = listSchemas.next() )
        pschema->selectionChanged( p );

    if ( p == this )
    {
    }
}



