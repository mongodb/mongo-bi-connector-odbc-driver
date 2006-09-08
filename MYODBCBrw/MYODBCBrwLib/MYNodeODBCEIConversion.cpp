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

#include "MYNodeODBCEIConversion.h"

MYNodeODBCEIConversion::MYNodeODBCEIConversion( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIConversion::MYNodeODBCEIConversion( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIConversion::MYNodeODBCEIConversion( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEIConversion::~MYNodeODBCEIConversion()
{
}

void MYNodeODBCEIConversion::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    SQLRETURN       nReturn;
    SQLUINTEGER     nBitMask;   

    // conversion
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_BIGINT" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_BIGINT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_BINARY" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_BINARY, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_BIT" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_BIT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_CHAR" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_CHAR, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_DATE" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_DATE, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_DECIMAL" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_DECIMAL, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_DOUBLE" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_DOUBLE, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_FLOAT" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_FLOAT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_INTEGER" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_INTEGER, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_INTERVAL_YEAR_MONTH" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_INTERVAL_YEAR_MONTH, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_INTERVAL_DAY_TIME" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_INTERVAL_DAY_TIME, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_LONGVARBINARY" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_LONGVARBINARY, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_LONGVARCHAR" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_LONGVARCHAR, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_NUMERIC" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_NUMERIC, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_REAL" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_REAL, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_SMALLINT" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_SMALLINT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_TIME" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_TIME, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_TIMESTAMP" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_TIMESTAMP, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_TINYINT" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_TINYINT, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_VARBINARY" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_VARBINARY, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_VARCHAR" );
    pnodeextendedinfoitem->setText( 2, "indicates the conversions supported by the data source with the CONVERT scalar function" );
    nReturn = pconnection->getInfo( SQL_CONVERT_VARCHAR, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( SQL_SUCCEEDED( nReturn ) )
        doLoadConversionDetails( nBitMask, pnodeextendedinfoitem );
}

void MYNodeODBCEIConversion::doLoadConversionDetails( SQLUINTEGER nBitMask, MYNodeODBCExtendedInfoItem *pnodeextendedinfoitem )
{
    MYNodeODBCExtendedInfoItem *pnodeextendedinfoitem2;

    pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_BIGINT" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_BIGINT )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_BINARY" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_BINARY )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_BIT" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_BIT )
        pnodeextendedinfoitem2->setText( 1, "Y" );

/*
    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_GUID" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_GUID )
        pnodeextendedinfoitem2->setText( 1, "Y" );
*/
    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_CHAR" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_CHAR )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_DATE" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_DATE )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_DECIMAL" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_DECIMAL )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_DOUBLE" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_DOUBLE )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_FLOAT" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_FLOAT )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_INTEGER" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_INTEGER )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_INTERVAL_YEAR_MONTH" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_INTERVAL_YEAR_MONTH )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_INTERVAL_DAY_TIME" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_INTERVAL_DAY_TIME )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_LONGVARBINARY" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_LONGVARBINARY )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_LONGVARCHAR" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_LONGVARCHAR )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_NUMERIC" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_NUMERIC )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_REAL" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_REAL )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_SMALLINT" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_SMALLINT )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_TIME" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_TIME )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_TIMESTAMP" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_TIMESTAMP )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_TINYINT" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_TINYINT )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_VARBINARY" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_VARBINARY )
        pnodeextendedinfoitem2->setText( 1, "Y" );

    pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
    pnodeextendedinfoitem2->setText( 0, "SQL_CVT_VARCHAR" );
    pnodeextendedinfoitem2->setText( 1, "N" );
    if ( nBitMask & SQL_CVT_VARCHAR )
        pnodeextendedinfoitem2->setText( 1, "Y" );
}

void MYNodeODBCEIConversion::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEIConversion::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "Conversion" );
    setText( 2, "The SQL data types to which the data source can convert the specified SQL data type with the CONVERT scalar function." );
}


