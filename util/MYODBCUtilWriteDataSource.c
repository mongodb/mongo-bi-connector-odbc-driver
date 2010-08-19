/* Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/ODBC is licensed under the terms of the GPLv2
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPLv2 as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details.
   
   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "MYODBCUtil.h"


/*!
  \brief    Creates/replaces the given data source.

            The use of SQLWriteDSNToIni() means this function
            does not really update an existing DSN so much as
            replaces it.
*/
BOOL MYODBCUtilWriteDataSource( MYODBCUTIL_DATASOURCE *pDataSource )
{
    SAVE_MODE();

    /*
        SQLWriteDSNToIni is *supposed* to replace any existing DSN
        with same name but fails (at least on unixODBC) to do so.
        So we ensure that any existing DSN with same name is removed
        with the following call.
    */
    if ( !SQLRemoveDSNFromIni( pDataSource->pszDSN ) )
        return FALSE;

    RESTORE_MODE();

    /* create/replace data source name */
    if ( !SQLWriteDSNToIni( pDataSource->pszDSN, pDataSource->pszDRIVER ) )
        return FALSE;

    RESTORE_MODE();

    /* A little helper to avoid duplicated code. */
#define WRITE_VALUE(field, name) \
    if (pDataSource->psz##field && \
        !SQLWritePrivateProfileString(pDataSource->pszDSN, name, \
                                      pDataSource->psz##field, "odbc.ini")) \
      return FALSE; \
    RESTORE_MODE()

    /* add details */
    WRITE_VALUE(DATABASE, "DATABASE");
    WRITE_VALUE(DESCRIPTION, "DESCRIPTION");
    WRITE_VALUE(OPTION, "OPTION");
    WRITE_VALUE(PASSWORD, "PWD");
    WRITE_VALUE(PORT, "PORT");
    WRITE_VALUE(SERVER, "SERVER");
    WRITE_VALUE(SOCKET, "SOCKET");
    WRITE_VALUE(STMT, "STMT");
    WRITE_VALUE(USER, "UID");
    WRITE_VALUE(SSLCA, "SSLCA");
    WRITE_VALUE(SSLCAPATH, "SSLCAPATH");
    WRITE_VALUE(SSLCERT, "SSLCERT");
    WRITE_VALUE(SSLCIPHER, "SSLCIPHER");
    WRITE_VALUE(SSLKEY, "SSLKEY");
    WRITE_VALUE(SSLVERIFY, "SSLVERIFY");
    WRITE_VALUE(CHARSET, "CHARSET");
    if (pDataSource->bINTERACTIVE
      && !SQLWritePrivateProfileString(pDataSource->pszDSN, "INTERACTIVE", "1", "odbc.ini"))
      return FALSE;

    return TRUE;
}
