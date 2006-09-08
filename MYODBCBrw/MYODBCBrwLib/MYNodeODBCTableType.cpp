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

#include "MYNodeODBCTableType.h"

#include "FolderClosed16x16.xpm"

MYNodeODBCTableType::MYNodeODBCTableType( QListView *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog, stringSchema, stringType );
}

MYNodeODBCTableType::MYNodeODBCTableType( QListViewItem *pParent, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType )
    : MYNode( pParent )
{
    Init( pconnection, stringCatalog, stringSchema, stringType );
}

MYNodeODBCTableType::MYNodeODBCTableType( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType )
    : MYNode( pParent, pAfter  )
{
    Init( pconnection, stringCatalog, stringSchema, stringType );
}

MYNodeODBCTableType::~MYNodeODBCTableType()
{
    listChildren.clear();
}

void MYNodeODBCTableType::setOpen( bool bOpen )
{
    QListViewItem::setOpen( bOpen );
    listView()->setSelected( listView()->selectedItem(), false );
    if ( bOpen )
    {
        doLoadChildren();
//        setSelected( true );
    }
    else
        listChildren.clear();
}

void MYNodeODBCTableType::selectionChanged( QListViewItem *p )
{
//    MYNodeODBCExtendedInfoItem    *ptype;

//    for ( ptype = listChildren.first(); ptype != 0; ptype = listChildren.next() )
//        ptype->selectionChanged( p );

    if ( p == this )
    {
    }
}


void MYNodeODBCTableType::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}


void MYNodeODBCTableType::doLoadChildren()
{
    MYQTODBCStatement *               pstatement = new MYQTODBCStatement( pconnection );
    SQLRETURN                        nReturn;
    SQLCHAR                         szTable[101];
    SQLCHAR                         szRemarks[301];
    SQLINTEGER                      nIndTable;
    SQLINTEGER                      nIndRemarks;
    MYNode *                       pnodeLast = NULL;

    nReturn = pstatement->getTables( stringCatalog, stringSchema, QString::null, stringType );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        return;
    }

    *szTable    = '\0';
    *szRemarks  = '\0';

    pstatement->doBindCol( 3, SQL_C_CHAR, szTable, sizeof(szTable) - 1, &nIndTable );
    pstatement->doBindCol( 5, SQL_C_CHAR, szRemarks, sizeof(szRemarks) - 1, &nIndRemarks );

    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
        if ( stringType == "TABLE" || stringType == "SYSTEM TABLE" || stringType == "VIEW" )
        {
            MYNodeODBCTable *ptable = new MYNodeODBCTable( this, pnodeLast, pconnection, stringCatalog, stringSchema, (char *)szTable, stringType, (char*)szRemarks );
            pnodeLast = ptable;
            listChildren.append( ptable );
        }
        else // "GLOBAL TEMPORARY", "LOCAL TEMPORARY", "ALIAS", "SYNONYM", and driver specific
        {
            MYNodeODBCExtendedInfoItem * ptype = new MYNodeODBCExtendedInfoItem( this, pnodeLast );
            ptype->setText( 0, (const char *)szTable );
            ptype->setText( 1, stringType );
            ptype->setText( 2, (const char *)szRemarks );
            listChildren.append( ptype );
        }

        *szTable    = '\0';
        *szRemarks  = '\0';
    }

    delete pstatement;
}

void MYNodeODBCTableType::Init( MYQTODBCConnection *pconnection, const QString &stringCatalog, const QString &stringSchema, const QString &stringType )
{
    this->pconnection        = pconnection;
    this->stringCatalog     = stringCatalog;
    this->stringSchema      = stringSchema;
    this->stringType        = stringType;
    setText( 0, stringType );
    setText( 1, "TABLE TYPE" );
    setText( 2, "" );
    listChildren.setAutoDelete( TRUE );
    setPixmap( 0, QPixmap( xpmFolderClosed16x16 ) );
}


