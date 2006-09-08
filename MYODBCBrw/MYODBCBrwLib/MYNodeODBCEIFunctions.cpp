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

#include "MYNodeODBCEIFunctions.h"

MYNodeODBCEIFunctions::MYNodeODBCEIFunctions( QListView *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIFunctions::MYNodeODBCEIFunctions( QListViewItem *pParent, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent )
{
    Init( pconnection );
}

MYNodeODBCEIFunctions::MYNodeODBCEIFunctions( QListViewItem *pParent, QListViewItem *pAfter, MYQTODBCConnection *pconnection )
    : MYNodeFolder( pParent, pAfter  )
{
    Init( pconnection );
}

MYNodeODBCEIFunctions::~MYNodeODBCEIFunctions()
{
}

void MYNodeODBCEIFunctions::setOpen( bool bOpen )
{
    MYNodeFolder::setOpen( bOpen );
    if ( !bOpen || firstChild() )
        return;

    // LOAD CHILDREN
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem;
    MYNodeODBCExtendedInfoItem * pnodeextendedinfoitem2;
    SQLRETURN       nReturn;
    SQLUINTEGER     nBitMask;   

    // scalar functions
    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_CONVERT_FUNCTIONS" );
    pnodeextendedinfoitem->setText( 2, "the scalar conversion functions supported by the driver and associated data source" );
    nReturn = pconnection->getInfo( SQL_CONVERT_FUNCTIONS, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_CVT_CAST" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_CVT_CAST )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_CVT_CONVERT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_CVT_CONVERT )
            pnodeextendedinfoitem2->setText( 1, "Y" );
    }

    pnodeextendedinfoitem = new MYNodeODBCExtendedInfoItem( this );
    pnodeextendedinfoitem->setText( 0, "SQL_NUMERIC_FUNCTIONS" );
    pnodeextendedinfoitem->setText( 2, "the scalar numeric functions supported by the driver and associated data source" );
    nReturn = pconnection->getInfo( SQL_NUMERIC_FUNCTIONS, (SQLPOINTER)&nBitMask, sizeof(nBitMask), NULL );
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
    {
        pnodeextendedinfoitem->setText( 1, QString::number( nBitMask ) );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_ABS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_ABS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_ACOS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_ACOS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_ASIN" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_ASIN )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_ATAN" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_ATAN )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_ATAN2" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_ATAN2 )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_CEILING" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_CEILING )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_COS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_COS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_COT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_COT )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_DEGREES" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_DEGREES )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_EXP" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_EXP )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_FLOOR" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_FLOOR )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_LOG" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_LOG )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_LOG10" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_LOG10 )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_MOD" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_MOD )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_PI" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_PI )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_POWER" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_POWER )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_RADIANS" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_RADIANS )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_RAND" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_RAND )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_ROUND" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_ROUND )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_SIGN" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_SIGN )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_SIN" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_SIN )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_SQRT" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_SQRT )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_TAN" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_TAN )
            pnodeextendedinfoitem2->setText( 1, "Y" );

        pnodeextendedinfoitem2 = new MYNodeODBCExtendedInfoItem( pnodeextendedinfoitem );
        pnodeextendedinfoitem2->setText( 0, "SQL_FN_NUM_TRUNCATE" );
        pnodeextendedinfoitem2->setText( 1, "N" );
        pnodeextendedinfoitem2->setText( 2, "" );
        if ( nBitMask & SQL_FN_NUM_TRUNCATE )
            pnodeextendedinfoitem2->setText( 1, "Y" );

    }


/*  
SQL_STRING_FUNCTIONS  
SQL_SYSTEM_FUNCTIONS 
SQL_TIMEDATE_ADD_INTERVALS
SQL_TIMEDATE_DIFF_INTERVALS
SQL_TIMEDATE_FUNCTIONS
*/

}

void MYNodeODBCEIFunctions::setup()
{
    // plus/minus by default (even when no child items)
    setExpandable( true );
    QListViewItem::setup();
}

void MYNodeODBCEIFunctions::Init( MYQTODBCConnection *pconnection )
{
    this->pconnection        = pconnection;
    setText( 0, "Scalar Functions" );
    setText( 2, "Information about the scalar functions supported by the data source and the driver." );
}


