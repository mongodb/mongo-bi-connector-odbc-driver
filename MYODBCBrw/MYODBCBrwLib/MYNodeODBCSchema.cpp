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

#include "MYNodeODBCSchema.h"

#include "FolderClosed16x16.xpm"

MYNodeODBCSchema::MYNodeODBCSchema( QListView *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog, stringSchema );
}

MYNodeODBCSchema::MYNodeODBCSchema( QListViewItem *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog, stringSchema );
}

MYNodeODBCSchema::MYNodeODBCSchema( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema )
    : MYNode( pParent, pAfter  )
{
    Init( pconnection, stringCatalog, stringSchema );
}

MYNodeODBCSchema::~MYNodeODBCSchema()
{
    Fini();
}

void MYNodeODBCSchema::Init( MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema )
{
    this->pconnection   = pconnection;
    this->stringCatalog = stringCatalog;
    this->stringSchema  = stringSchema;
    setText( 0, stringSchema );
    setText( 1, "SCHEMA (namespace)" );
    sortChildItems( 1, FALSE );
    listTypes.setAutoDelete( TRUE );
    setPixmap( 0, QPixmap( xpmFolderClosed16x16 ) );
}

void MYNodeODBCSchema::Fini()
{
    listTypes.clear();
}

void MYNodeODBCSchema::setOpen( bool bOpen )
{
    QListViewItem::setOpen( bOpen );
    listView()->setSelected( listView()->selectedItem(), false );
    if ( bOpen )
    {
        doLoadTableTypes();
        // If have less than 1 children than we know that we failed to find any table types
        // so create defaults.
        if ( listTypes.count() < 1 )
        {
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "TABLE" ) );
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "VIEW" ) );
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "SYSTEM TABLE" ) );
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "GLOBAL TEMPORARY" ) );
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "LOCAL TEMPORARY" ) );
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "ALIAS" ) );
            listTypes.append( new MYNodeODBCTableType( this, NULL, pconnection, stringCatalog, stringSchema, "SYNONYM" ) );
        }
    }
    else
    {
        Fini();
    }
}

void MYNodeODBCSchema::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}

/*!
    doLoadTableTypes
    
*/    
void MYNodeODBCSchema::doLoadTableTypes()
{
    MYQTODBCStatement *       pstatement  = new MYQTODBCStatement( pconnection );
    SQLRETURN                nReturn;
    SQLCHAR                 szType[101];
    SQLINTEGER              nType;
    MYNodeODBCTableType *  ptype = NULL;

    if ( !SQL_SUCCEEDED( pstatement->getTables( QString::null, QString::null, QString::null, SQL_ALL_TABLE_TYPES ) ) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        *szType = '\0';

        nReturn = pstatement->getData( 4, SQL_C_CHAR, szType, sizeof(szType) - 1, &nType );
        if ( SQL_SUCCEEDED( nReturn ) && nType != SQL_NULL_DATA && *szType != '\0' )
            listTypes.append( ptype = new MYNodeODBCTableType( this, ptype, pconnection, stringCatalog, stringSchema, (const char *)szType ) );
    }

    delete pstatement;
}

void MYNodeODBCSchema::selectionChanged( QListViewItem *p )
{
    MYNodeODBCTableType * ptype;

    for ( ptype = listTypes.first(); ptype != 0; ptype = listTypes.next() )
        ptype->selectionChanged( p );

    if ( p == this )
    {
    }
}



