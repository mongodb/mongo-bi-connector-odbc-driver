/*
  Copyright (C) 2007 MySQL AB

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

/**
 @file  odbcinstw.c
 @brief Installer API Unicode wrapper functions.

 unixODBC 2.2.11 does not include the Unicode versions of the ODBC
 installer API. As of November 2007, this was still the current version
 shipped with Debian and Ubuntu Linux.

 SQLGetPrivateProfileString() also has a few bugs in unixODBC 2.2.11 that
 our version of SQLGetPrivateProfileStringW() will work around.
*/

#include "stringutil.h"
#include "../MYODBC_CONF.h"
#include "../MYODBC_ODBC.h"

#include <sql.h>
#include <wchar.h>

#define INSTAPI

#ifndef FALSE
# define FALSE 0
#endif


#if !defined(HAVE_SQLGETPRIVATEPROFILESTRINGW) || defined(USE_UNIXODBC)
int INSTAPI
MySQLGetPrivateProfileStringW(const LPWSTR lpszSection, const LPWSTR lpszEntry,
                              const LPWSTR lpszDefault, LPWSTR lpszRetBuffer,
                              int cbRetBuffer, const LPWSTR lpszFilename)
{
  SQLINTEGER len;
  int rc;
  char *section, *entry, *def, *ret, *filename;

  len= SQL_NTS;
  section= (char *)sqlwchar_as_utf8(lpszSection, &len);
  len= SQL_NTS;
  entry= (char *)sqlwchar_as_utf8(lpszEntry, &len);
  len= SQL_NTS;
  def= (char *)sqlwchar_as_utf8(lpszDefault, &len);
  len= SQL_NTS;
  filename= (char *)sqlwchar_as_utf8(lpszFilename, &len);

  if (lpszRetBuffer && cbRetBuffer)
    ret= malloc(cbRetBuffer + 1);
  else
    ret= NULL;

  /* unixODBC 2.2.11 can't handle NULL for default, so pass "" instead. */
  rc= SQLGetPrivateProfileString(section, entry, def ? def : "", ret,
                                 cbRetBuffer, filename);

  if (rc > 0 && lpszRetBuffer)
  {
    /*
     unixODBC 2.2.11 returns the wrong value from SQLGetPrivateProfileString
     when getting the list of sections or entries in a section, so we have to
     re-calculate the correct length by walking the list of values.
    */
    if (!entry || !section)
    {
      char *pos= ret;
      while (*pos && pos < ret + cbRetBuffer)
        pos+= strlen(pos) + 1;
      rc= pos - ret;
    }

    /** @todo error handling */
    utf8_as_sqlwchar(lpszRetBuffer, cbRetBuffer, (SQLCHAR *)ret, rc);
  }

  x_free(section);
  x_free(entry);
  x_free(def);
  x_free(ret);
  x_free(filename);

  return rc;
}
#endif


#ifndef HAVE_SQLGETPRIVATEPROFILESTRINGW
int INSTAPI
SQLGetPrivateProfileStringW(const LPWSTR lpszSection, const LPWSTR lpszEntry,
                            const LPWSTR lpszDefault, LPWSTR lpszRetBuffer,
                            int cbRetBuffer, const LPWSTR lpszFilename)
{
  return MySQLGetPrivateProfileStringW(lpszSection, lpszEntry, lpszDefault,
                                       lpszRetBuffer, cbRetBuffer,
                                       lpszFilename);
}

/**
  The version of iODBC that shipped with Mac OS X 10.4 does not implement
  the Unicode versions of various installer API functions, so we have to
  do it ourselves.
*/

BOOL INSTAPI
SQLInstallDriverExW(const LPWSTR lpszDriver, const LPWSTR lpszPathIn,
                    LPWSTR lpszPathOut, WORD cbPathOutMax, WORD *pcbPathOut,
                    WORD fRequest, LPDWORD lpdwUsageCount)
{
  LPWSTR pos;
  SQLINTEGER len;
  BOOL rc;
  char *driver, *pathin, *pathout;
  WORD out;

  if (!pcbPathOut)
    pcbPathOut= &out;

  /* Calculate length of double-\0 terminated string */
  pos= lpszDriver;
  while (*pos)
    pos+= sqlwcharlen(pos) + 1;
  len= pos - lpszDriver + 1;
  driver= (char *)sqlwchar_as_utf8(lpszDriver, &len);

  len= SQL_NTS;
  pathin= (char *)sqlwchar_as_utf8(lpszPathIn, &len);

  if (cbPathOutMax > 0)
    pathout= (char *)malloc(cbPathOutMax * 4 + 1); /* 4 = max utf8 charlen */

  rc= SQLInstallDriverEx(driver, pathin, pathout, cbPathOutMax * 4,
                         pcbPathOut, fRequest, lpdwUsageCount);

  if (rc == TRUE && cbPathOutMax)
    *pcbPathOut= utf8_as_sqlwchar(lpszPathOut, cbPathOutMax,
                                  (SQLCHAR *)pathout, *pcbPathOut);

  x_free(driver);
  x_free(pathin);
  x_free(pathout);

  return rc;
}


BOOL INSTAPI
SQLValidDSNW(const LPWSTR lpszDSN)
{
  BOOL ret;
  SQLINTEGER len= SQL_NTS;
  char *dsn= (char *)sqlwchar_as_utf8(lpszDSN, &len);

  ret= SQLValidDSN(dsn);

  x_free(dsn);

  return ret;
}


BOOL INSTAPI
SQLRemoveDSNFromIniW(const LPWSTR lpszDSN)
{
  BOOL ret;
  SQLINTEGER len= SQL_NTS;
  char *dsn= (char *)sqlwchar_as_utf8(lpszDSN, &len);

  ret= SQLRemoveDSNFromIni(dsn);

  x_free(dsn);

  return ret;
}


BOOL INSTAPI
SQLWriteDSNToIniW(const LPWSTR lpszDSN, const LPWSTR lpszDriver)
{
  BOOL ret;
  SQLINTEGER len;
  char *dsn= NULL, *driver= NULL;

  len= SQL_NTS;
  dsn= (char *)sqlwchar_as_utf8(lpszDSN, &len),

  len= SQL_NTS;
  driver= (char *)sqlwchar_as_utf8(lpszDriver, &len);

  ret= SQLWriteDSNToIni(dsn, driver);

  x_free(dsn);
  x_free(driver);

  return ret;
}


RETCODE INSTAPI
SQLPostInstallerErrorW(DWORD fErrorCode, LPWSTR szErrorMsg)
{
  RETCODE ret;
  SQLINTEGER len= SQL_NTS;
  char *msg= (char *)sqlwchar_as_utf8(szErrorMsg, &len);

  ret= SQLPostInstallerError(fErrorCode, msg);

  /*
    We have to leak memory here, because iODBC does not make a
    copy of the message.
  */

  return ret;
}


BOOL INSTAPI
SQLRemoveDriverW(const LPWSTR lpszDriver, BOOL fRemoveDSN,
                 LPDWORD lpdwUsageCount)
{
  BOOL ret;
  SQLINTEGER len= SQL_NTS;
  char *driver= (char *)sqlwchar_as_utf8(lpszDriver, &len);

  ret= SQLRemoveDriver(driver, fRemoveDSN, lpdwUsageCount);

  x_free(driver);

  return ret;
}


BOOL INSTAPI
SQLWritePrivateProfileStringW(const LPWSTR lpszSection, const LPWSTR lpszEntry,
                              const LPWSTR lpszString,
                              const LPWSTR lpszFilename)
{
  BOOL ret;
  SQLINTEGER len;
  char *section= NULL, *entry= NULL, *string= NULL, *filename= NULL;

  len= SQL_NTS;
  section= (char *)sqlwchar_as_utf8(lpszSection, &len),
  len= SQL_NTS;
  entry= (char *)sqlwchar_as_utf8(lpszEntry, &len),
  len= SQL_NTS;
  string= (char *)sqlwchar_as_utf8(lpszString, &len),
  len= SQL_NTS;
  filename= (char *)sqlwchar_as_utf8(lpszFilename, &len),

  ret= SQLWritePrivateProfileString(section, entry, string, filename);

  x_free(section);
  x_free(entry);
  x_free(string);
  x_free(filename);

  return ret;
}
#endif
