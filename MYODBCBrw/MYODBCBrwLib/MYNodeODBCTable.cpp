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

#include "MYNodeODBCTable.h"

#include "Table16x16.xpm"

MYNodeODBCTable::MYNodeODBCTable( QListView *pParent, 
                                    MYQTODBCConnection *pconnection,
                                    const QString &stringCatalog, 
                                    const QString &stringSchema, 
                                    const QString &stringTable, 
                                    const QString &stringType, 
                                    const QString &stringRemark )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog, stringSchema, stringTable, stringType, stringRemark );
}

MYNodeODBCTable::MYNodeODBCTable( QListViewItem *pParent, 
                                    MYQTODBCConnection *pconnection,
                                    const QString &stringCatalog, 
                                    const QString &stringSchema, 
                                    const QString &stringTable, 
                                    const QString &stringType, 
                                    const QString &stringRemark )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog, stringSchema, stringTable, stringType, stringRemark );
}

MYNodeODBCTable::MYNodeODBCTable( QListViewItem *pParent, QListViewItem *pAfter, 
                                    MYQTODBCConnection *pconnection,
                                    const QString &stringCatalog, 
                                    const QString &stringSchema, 
                                    const QString &stringTable, 
                                    const QString &stringType, 
                                    const QString &stringRemark )
    : MYNode( pParent, pAfter  )
{
    Init( pconnection, stringCatalog, stringSchema, stringTable, stringType, stringRemark );
}

MYNodeODBCTable::~MYNodeODBCTable()
{
    Fini();
}

void MYNodeODBCTable::Init( MYQTODBCConnection *pconnection, 
                             const QString &stringCatalog, 
                             const QString &stringSchema, 
                             const QString &stringTable, 
                             const QString &stringType, 
                             const QString &stringRemarks )
{
    this->pconnection   = pconnection;
    this->stringCatalog = stringCatalog;
    this->stringSchema  = stringSchema;
    setText( 0, stringTable );
    setText( 1, stringType );
    setText( 2, stringRemarks );
    sortChildItems( 1, FALSE );
    listColumns.setAutoDelete( TRUE );
    pPrimaryKeys    = 0;
    pIndexs         = 0;
    pSpecialColumns    = 0;
    setPixmap( 0, QPixmap( xpmTable16x16 ) );
}

void MYNodeODBCTable::Fini()
{
    listColumns.clear();
    if ( pPrimaryKeys ) delete pPrimaryKeys;
    if ( pIndexs ) delete pIndexs;
    if ( pSpecialColumns ) delete pSpecialColumns;
    pPrimaryKeys    = 0;
    pIndexs         = 0;
    pSpecialColumns    = 0;
}

void MYNodeODBCTable::setOpen( bool bOpen )
{
    QListViewItem::setOpen( bOpen );
    listView()->setSelected( listView()->selectedItem(), false );
    if ( bOpen )
    {
        doLoadColumns();
        QString stringTable= text( 0 );
        QString stringType = text( 1 );
        if ( stringType == "TABLE" || stringType == "SYSTEM TABLE" )
        {
            pPrimaryKeys    = new MYNodeODBCPrimaryKeys( this, pconnection, stringTable );
            pIndexs         = new MYNodeODBCIndexs( this, pconnection, stringTable );
            pSpecialColumns = new MYNodeODBCSpecialColumns( this, pconnection, stringTable );
        }
//        listView()->setSelected( listView()->selectedItem(), false );
//        setSelected( true );
    }
    else
    {
        Fini();
    }
}

void MYNodeODBCTable::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}

void MYNodeODBCTable::doLoadColumns()
{
    MYQTODBCStatement *   pstatement  = new MYQTODBCStatement( pconnection );
    SQLRETURN            nReturn;
    SQLCHAR             szColumn[101];
    SQLCHAR             szType[101];
    SQLCHAR             szRemark[301];
    MYNodeODBCColumn * pColumn = NULL;
    QString             stringTable = text( 0 );
    SQLINTEGER          nIndicator;

    if ( !SQL_SUCCEEDED( pstatement->getColumns( stringCatalog, stringSchema, stringTable ) ) )
    {
        delete pstatement;
        return;
    }

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        szColumn[0] = '\0';
        szType[0]   = '\0';
        szRemark[0] = '\0';

        nReturn = pstatement->getData( 4, SQL_C_CHAR, szColumn, sizeof(szColumn), &nIndicator );
        nReturn = pstatement->getData( 6, SQL_C_CHAR, szType, sizeof(szType), &nIndicator );

        listColumns.append( pColumn = new MYNodeODBCColumn( this, pColumn, pconnection, (char *)szColumn, (char*)szType, (char*)szRemark ) );

    }

    delete pstatement;
}

void MYNodeODBCTable::selectionChanged( QListViewItem *p )
{
    MYNodeODBCColumn    *pColumn;

    for ( pColumn = listColumns.first(); pColumn != 0; pColumn = listColumns.next() )
        pColumn->selectionChanged( p );

    if ( p == this )
    {
    }
}



