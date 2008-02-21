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
    \brief  Set NULL values to default value.

    \param  pDataSource Data Source struct.

    \note   The default values have been gleaned from pre-existing 
            code in connect.c but there are some inconsistencies
            between SQLConnect and SQLDriverConnect so I assume that
            mysql_real_connect is smarter than it used to be.
            This needs to be cleaned up such that it reflects what is
            really needed.
    
*/            
BOOL MYODBCUtilDefaultDataSource( MYODBCUTIL_DATASOURCE *pDataSource )
{
    if ( pDataSource->pszSERVER && !pDataSource->pszSERVER[0] )
    {
        _global_free( pDataSource->pszSERVER );
        pDataSource->pszSERVER = NULL;
    }

    if ( pDataSource->pszDATABASE && !pDataSource->pszDATABASE[0] )
    {
        _global_free( pDataSource->pszDATABASE );
        pDataSource->pszDATABASE = NULL;
    }

    if ( pDataSource->pszUSER && !pDataSource->pszUSER[0] )
    {
        _global_free( pDataSource->pszUSER );
        pDataSource->pszUSER = NULL;
    }

    if ( pDataSource->pszPASSWORD && !pDataSource->pszPASSWORD[0] )
    {
        _global_free( pDataSource->pszPASSWORD );
        pDataSource->pszPASSWORD = NULL;
    }

    if ( !pDataSource->pszPORT )
        pDataSource->pszPORT = _global_strdup( "0" );

    if ( !pDataSource->pszOPTION )
        pDataSource->pszOPTION = _global_strdup( "0" );

	if( pDataSource->pszSSLCA && !pDataSource->pszSSLCA[0] )
	{
		_global_free (pDataSource->pszSSLCA);
		pDataSource->pszSSLCA = NULL;
	}
	if( pDataSource->pszSSLCAPATH && !pDataSource->pszSSLCAPATH[0] )
	{
		_global_free (pDataSource->pszSSLCAPATH);
		pDataSource->pszSSLCAPATH = NULL;
	}
	if( pDataSource->pszSSLCERT && !pDataSource->pszSSLCERT[0] )
	{
		_global_free (pDataSource->pszSSLCERT);
		pDataSource->pszSSLCERT = NULL;
	}
	if( pDataSource->pszSSLCIPHER && !pDataSource->pszSSLCIPHER[0] )
	{
		_global_free (pDataSource->pszSSLCIPHER);
		pDataSource->pszSSLCIPHER = NULL;
	}
	if( pDataSource->pszSSLKEY && !pDataSource->pszSSLKEY[0] )
	{
		_global_free (pDataSource->pszSSLKEY);
		pDataSource->pszSSLKEY = NULL;
	}
    if( pDataSource->pszSSLVERIFY && !pDataSource->pszSSLVERIFY[0] )
    {
        _global_free (pDataSource->pszSSLVERIFY);
        pDataSource->pszSSLVERIFY = NULL;
    }
	if (pDataSource->pszCHARSET && !pDataSource->pszCHARSET[0] )
	{
          _global_free(pDataSource->pszCHARSET);
          pDataSource->pszCHARSET= NULL;
	}

#ifndef _UNIX_
    /* Here we actually unset socket for non-UNIX as it does not apply. */
    if ( pDataSource->pszSOCKET && !pDataSource->pszSOCKET[0] )
    {
        free( pDataSource->pszSOCKET );
        pDataSource->pszSOCKET = NULL;
    }
#endif

    return TRUE;
}


