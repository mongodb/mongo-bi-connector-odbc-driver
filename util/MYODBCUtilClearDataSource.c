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

#include "MYODBCUtil.h"

/*!
  \brief    Reinitializes MYODBCUTIL_DATASOURCE.

  \note     Does NOT (read; MUST NOT) reinit pszDriverFileName, pszDRIVER, 
            nMode, nConnect, and nPrompt.
  
*/  

void MYODBCUtilClearDataSource( MYODBCUTIL_DATASOURCE *pDataSource )
{
    if ( !pDataSource )
        return;

    if ( pDataSource->pszDATABASE )
    {
		_global_free( pDataSource->pszDATABASE );
        pDataSource->pszDATABASE = NULL;
    }
    if ( pDataSource->pszDESCRIPTION )
    {
        _global_free( pDataSource->pszDESCRIPTION );
        pDataSource->pszDESCRIPTION = NULL;
    }
    if ( pDataSource->pszDSN )
    {
        _global_free( pDataSource->pszDSN );
        pDataSource->pszDSN = NULL;
    }
    if ( pDataSource->pszOPTION )
    {
        _global_free( pDataSource->pszOPTION );
        pDataSource->pszDSN = NULL;
    }
    if ( pDataSource->pszPASSWORD )
    {
        _global_free( pDataSource->pszPASSWORD );
        pDataSource->pszPASSWORD = NULL;
    }
    if ( pDataSource->pszPORT )
    {
        _global_free( pDataSource->pszPORT );
        pDataSource->pszPORT = NULL;
    }
    if ( pDataSource->pszSERVER )
    {
        _global_free( pDataSource->pszSERVER );
        pDataSource->pszSERVER = NULL;
    }
    if ( pDataSource->pszSOCKET )
    {
        _global_free( pDataSource->pszSOCKET );
        pDataSource->pszSOCKET = NULL;
    }
    if ( pDataSource->pszSTMT )
    {
        _global_free( pDataSource->pszSTMT );
        pDataSource->pszSTMT = NULL;
    }
    if ( pDataSource->pszUSER )
    {
        _global_free( pDataSource->pszUSER );
        pDataSource->pszUSER = NULL;
    }
}


