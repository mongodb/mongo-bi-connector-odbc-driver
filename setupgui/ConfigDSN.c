/*
  Copyright (C) 2000-2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "setupgui.h"
#include "stringutil.h"

/*
   Entry point for GUI prompting from SQLDriverConnect().
*/
BOOL Driver_Prompt(HWND hWnd, SQLWCHAR *instr, SQLUSMALLINT completion,
                   SQLWCHAR *outstr, SQLSMALLINT outmax, SQLSMALLINT *outlen)
{
  DataSource *ds= ds_new();
  BOOL rc= FALSE;

  /*
     parse the attr string, dsn lookup will have already been
     done in the driver
  */
  if (instr && *instr)
  {
    if (ds_from_kvpair(ds, instr, (SQLWCHAR)';'))
    {
      rc= FALSE;
      goto exit;
    }
  }

  /* Show the dialog and handle result */
  if (ShowOdbcParamsDialog(ds, hWnd, TRUE) == 1)
  {
    int len;
    /* serialize to outstr */
    if ((len= ds_to_kvpair(ds, outstr, outmax, (SQLWCHAR)';')) == -1)
    {
      /* truncated, up to caller to see outmax < *outlen */
      if (outlen)
        *outlen= ds_to_kvpair_len(ds);
      outstr[outmax]= 0;
    }
    else if (outlen)
      *outlen= len;
    rc= TRUE;
  }

exit:
  ds_delete(ds);
  return rc;
}


/*
   Add, edit, or remove a Data Source Name (DSN). This function is
   called by "Data Source Administrator" on Windows, or similar
   application on Unix.
*/
BOOL INSTAPI ConfigDSNW(HWND hWnd, WORD nRequest, LPCWSTR pszDriver,
                        LPCWSTR pszAttributes)
{
  DataSource *ds= ds_new();
  BOOL rc= TRUE;
  Driver *driver= NULL;
  SQLWCHAR *origdsn= NULL;

  if (pszAttributes && *pszAttributes)
  {
    SQLWCHAR delim= ';';

    /* if there's no ;, then it's most likely null-delimited */
    if (!sqlwcharchr(pszAttributes, delim))
      delim= 0;

    if (ds_from_kvpair(ds, pszAttributes, delim))
    {
      SQLPostInstallerError(ODBC_ERROR_INVALID_KEYWORD_VALUE,
                            W_INVALID_ATTR_STR);
      rc= FALSE;
      goto exitConfigDSN;
    }
    if (ds_lookup(ds) && nRequest != ODBC_ADD_DSN)
    {
      /* ds_lookup() will already set SQLInstallerError */
      rc= FALSE;
      goto exitConfigDSN;
    }
    origdsn= sqlwchardup(ds->name, SQL_NTS);
  }

  switch (nRequest)
  {
  case ODBC_ADD_DSN:
    driver= driver_new();
    memcpy(driver->name, pszDriver,
           (sqlwcharlen(pszDriver) + 1) * sizeof(SQLWCHAR));
    if (driver_lookup(driver))
    {
      rc= FALSE;
      break;
    }
    ds_set_strattr(&ds->driver, driver->lib);
  case ODBC_CONFIG_DSN:
#ifdef _WIN32
    /*
      for windows, if hWnd is NULL, we try to add the dsn
      with what information was given
    */
    if (!hWnd || ShowOdbcParamsDialog(ds, hWnd, FALSE) == 1)
#elif
    if (ShowOdbcParamsDialog(ds, hWnd, FALSE) == 1)
#endif
    {
      /* save datasource */
      if (ds_add(ds))
        rc= FALSE;
      /* if the name is changed, remove the old dsn */
      if (origdsn && memcmp(origdsn, ds->name,
                            (sqlwcharlen(origdsn) + 1) * sizeof(SQLWCHAR)))
        SQLRemoveDSNFromIni(origdsn);
    }
    break;
  case ODBC_REMOVE_DSN:
    if (SQLRemoveDSNFromIni(ds->name) != TRUE)
      rc= FALSE;
    break;
  }

exitConfigDSN:
  x_free(origdsn);
  ds_delete(ds);
  if (driver)
    driver_delete(driver);
  return rc;
}
