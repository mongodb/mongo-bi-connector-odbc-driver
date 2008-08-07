/* Copyright (C) 2000-2007 MySQL AB

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

/*!
  @brief  This program will aid installers when installing/uninstalling
          MyODBC.

          This program can register/deregister a myodbc driver and create
          a sample dsn. The key thing here is that it does this with 
          cross-platform code - thanks to the ODBC installer API. This is
          most useful to those creating installers (apps using myodbc or 
          myodbc itself).

          For example, this program is used in the postinstall script
          of the MyODBC for Mac OS X installer package. 
*/

/*
 * Installer utility application. Handles installing/removing/listing, etc
 * driver and data source entries in the system. See usage text for details.
 */

#include "MYODBC_MYSQL.h"
#include "installer.h"
#include "stringutil.h"

#include <stdio.h>
#include <stdlib.h>

const char *usage =
"+---                                                                   \n"
"| myodbc-installer v" MYODBC_VERSION "                                 \n"
"+---                                                                   \n"
"|                                                                      \n"
"| Description                                                          \n"
"|                                                                      \n"
"|    This program can be used to create, edit or remove a DSN. It      \n"
"|    can also be used to register or deregister a driver. In other     \n"
"|    words - it can be used to manage ODBC system information.         \n"
"|                                                                      \n"
"|    This operates consistently across platforms. This has been created\n"
"|    specifically for MySQL Connector/ODBC.                            \n"
"|                                                                      \n"
"| Syntax                                                               \n"
"|                                                                      \n"
"|    myodbc-installer <Object> <Action> [Options]                      \n"
"|                                                                      \n"
"| Object                                                               \n"
"|                                                                      \n"
"|     -d driver                                                        \n"
"|     -s datasource                                                    \n"
"|                                                                      \n"
"| Action                                                               \n"
"|                                                                      \n"
"|     -l list                                                          \n"
"|     -a add (add/update for data source)                              \n"
"|     -r remove                                                        \n"
"|                                                                      \n"
"| Options                                                              \n"
"|                                                                      \n"
"|     -n <name>                                                        \n"
"|     -t <attribute string>                                            \n"
/* Note: These scope values line up with the SQLSetConfigMode constants */
"|     -c0 both                                                         \n"
"|     -c1 user data source                                             \n"
"|     -c2 system data source (default)                                 \n"
"|                                                                      \n"
"| Examples                                                             \n"
"|                                                                      \n"
"|    List drivers                                                      \n"
"|        -d -l                                                         \n"
"|                                                                      \n"
"|    Register a driver (UNIX example)                                  \n"
"|        -d -a -n \"MySQL ODBC 5.1 Driver\" \\                         \n"
"|              -t \"DRIVER=/usr/lib/myodbc5.so;SETUP=/usr/lib/myodbc3S.so\"\n"
"|                                                                      \n"
"|    Register a driver (Windows example)                               \n"
"|        -d -a -n \"MySQL ODBC 5.1 Driver\" \\                         \n"
"|              -t \"DRIVER=myodbc5.dll;SETUP=myodbc5S.dll\"            \n"
"|                                                                      \n"
"|    Add a new data source name                                        \n"
"|        -s -a -c2 -n \"test\" \\                                      \n"
"|              -t \"DRIVER=MySQL ODBC 5.1 Driver;SERVER=localhost;DATABASE=test;UID=myid;PWD=mypwd\"\n"
"|                                                                      \n"
"|    List data source name attributes for 'test'                       \n"
"|        -s -l -c2 -n \"test\"                                         \n"
"+---                                                                   \n";

/* command line args */
#define OBJ_DRIVER 'd'
#define OBJ_DATASOURCE 's'

#define ACTION_LIST 'l'
#define ACTION_ADD 'a'
#define ACTION_REMOVE 'r'

#define OPT_NAME 'n'
#define OPT_ATTR 't'
#define OPT_SCOPE 'c'

static char obj= 0;
static char action= 0;
static UWORD scope= ODBC_SYSTEM_DSN; /* default = system */

/* these two are 'paired' and the char data is converted to the wchar buf.
 * we check the char * to see if the arg was given then use the wchar in the
 * installer api. */
static char *name= NULL;
static SQLWCHAR *wname;

static char *attrstr= NULL;
static SQLWCHAR *wattrs;


void object_usage()
{
  fprintf(stderr,
          "[ERROR] Object must be either driver (d) or data source (s)\n");
}


void action_usage()
{
  fprintf(stderr, "[ERROR] One action must be specified\n");
}


void main_usage()
{
  fprintf(stderr, usage);
}


/*
 * Print an error retrieved from SQLInstallerError.
 */
void print_installer_error()
{
  int msgno;
  DWORD errcode;
  char errmsg[256];
  for(msgno= 1; msgno < 9; ++msgno)
  {
    if (SQLInstallerError(msgno, &errcode, errmsg, 256, NULL) != SQL_SUCCESS)
      return;
    fprintf(stderr, "[ERROR] SQLInstaller error %d: %s\n", errcode, errmsg);
  }
}


/*
 * Print a general ODBC error (non-odbcinst).
 */
void print_odbc_error(SQLHANDLE hnd, SQLSMALLINT type)
{
  SQLCHAR sqlstate[20];
  SQLCHAR errmsg[1000];
  SQLINTEGER nativeerr;

  fprintf(stderr, "[ERROR] ODBC error");

  if(SQL_SUCCEEDED(SQLGetDiagRec(type, hnd, 1, sqlstate, &nativeerr,
                                 errmsg, 1000, NULL)))
    printf(": [%s] %d -> %s\n", sqlstate, nativeerr, errmsg);
  printf("\n");
}


/*
 * Handler for "list driver" command (-d -l -n drivername)
 */
int list_driver_details(Driver *driver)
{
  SQLWCHAR buf[50000];
  SQLWCHAR *entries= buf;
  int rc;

  /* lookup the driver */
  if ((rc= driver_lookup(driver)) < 0)
  {
    fprintf(stderr, "[ERROR] Driver not found '%s'\n",
            ds_get_utf8attr(driver->name, &driver->name8));
    return 1;
  }
  else if (rc > 0)
  {
    print_installer_error();
    return 1;
  }

  /* print driver details */
  printf("FriendlyName: %s\n", ds_get_utf8attr(driver->name, &driver->name8));
  printf("DRIVER      : %s\n", ds_get_utf8attr(driver->lib, &driver->lib8));
  printf("SETUP       : %s\n", ds_get_utf8attr(driver->setup_lib,
                                               &driver->setup_lib8));

  return 0;
}


/*
 * Handler for "list drivers" command (-d -l)
 */
int list_drivers()
{
  char buf[50000];
  char *drivers= buf;

  if (!SQLGetInstalledDrivers(buf, 50000, NULL))
  {
    print_installer_error();
    return 1;
  }

  while (*drivers)
  {
    printf("%s\n", drivers);
    drivers += strlen(drivers) + 1;
  }

  return 0;
}


/*
 * Handler for "add driver" command (-d -a -n drivername -t attrs)
 */
int add_driver(Driver *driver, const SQLWCHAR *attrs)
{
  DWORD usage_count;
  SQLWCHAR attrs_null[4096]; /* null-delimited pairs */
  SQLWCHAR prevpath[256];

  /* read driver attributes into object */
  if (driver_from_kvpair_semicolon(driver, attrs))
  {
    fprintf(stderr,
            "[ERROR] Could not parse key-value pair attribute string\n");
    return 1;
  }

  /* convert to null-delimited for installer API */
  if (driver_to_kvpair_null(driver, attrs_null, 4096))
  {
    fprintf(stderr,
            "[ERROR] Could not create new key-value pair attribute string\n");
    return 1;
  }

  if (SQLInstallDriverExW(attrs_null, NULL, prevpath, 256, NULL,
                          ODBC_INSTALL_COMPLETE, &usage_count) != TRUE)
  {
    print_installer_error();
    return 1;
  }

  printf("Success: Usage count is %d\n", usage_count);

  return 0;
}


/*
 * Handler for "remove driver" command (-d -a -n drivername)
 */
int remove_driver(Driver *driver)
{
  DWORD usage_count;

  if (SQLRemoveDriverW(driver->name, FALSE, &usage_count) != TRUE)
  {
    print_installer_error();
    return 1;
  }

  printf("Success: Usage count is %d\n", usage_count);

  return 0;
}


/*
 * Handle driver actions. We setup a driver object to be used
 * for all actions.
 */
int handle_driver_action()
{
  int rc= 0;
  UWORD config_mode= config_set(scope);
  Driver *driver= driver_new();

  /* check name is given if needed */
  switch (action)
  {
  case ACTION_ADD:
  case ACTION_REMOVE:
    if (!name)
    {
      fprintf(stderr, "[ERROR] Name missing to add/remove driver\n");
      rc= 1;
      goto end;
    }
  }

  /* set the driver name */
  if (name)
    sqlwcharncpy(driver->name, wname, ODBCDRIVER_STRLEN);

  /* perform given action */
  switch (action)
  {
  case ACTION_LIST:
    if (name)
      rc= list_driver_details(driver);
    else
      rc= list_drivers();
    break;
  case ACTION_ADD:
    if (attrstr)
      rc= add_driver(driver, wattrs);
    else
    {
      fprintf(stderr, "[ERROR] Attribute string missing to add driver\n");
      rc= 1;
    }
    break;
  case ACTION_REMOVE:
    rc= remove_driver(driver);
    break;
  }

end:
  driver_delete(driver);
  config_set(config_mode);
  return rc;
}


/*
 * Handler for "list data source" command (-s -l -n drivername)
 */
int list_datasource_details(DataSource *ds)
{
  int rc;

  if ((rc= ds_lookup(ds)) < 0)
  {
    fprintf(stderr, "[ERROR] Data source not found '%s'\n",
            ds_get_utf8attr(ds->name, &ds->name8));
    return 1;
  }
  else if (rc > 0)
  {
    print_installer_error();
    return 1;
  }

  /* print the data source fields */
                       printf("Name:                %s\n", ds_get_utf8attr(ds->name, &ds->name8));
  if (ds->driver     ) printf("Driver:              %s\n", ds_get_utf8attr(ds->driver, &ds->driver8));
  if (ds->description) printf("Description:         %s\n", ds_get_utf8attr(ds->description, &ds->description8));
  if (ds->server     ) printf("Server:              %s\n", ds_get_utf8attr(ds->server, &ds->server8));
  if (ds->uid        ) printf("Uid:                 %s\n", ds_get_utf8attr(ds->uid, &ds->uid8));
  if (ds->pwd        ) printf("Pwd:                 %s\n", ds_get_utf8attr(ds->pwd, &ds->pwd8));
  if (ds->database   ) printf("Database:            %s\n", ds_get_utf8attr(ds->database, &ds->database8));
  if (ds->socket     ) printf("Socket:              %s\n", ds_get_utf8attr(ds->socket, &ds->socket8));
  if (ds->initstmt   ) printf("Initial statement:   %s\n", ds_get_utf8attr(ds->initstmt, &ds->initstmt8));
  if (ds->initstmt   ) printf("Option:              %s\n", ds_get_utf8attr(ds->option, &ds->option8));
  if (ds->initstmt   ) printf("Character set:       %s\n", ds_get_utf8attr(ds->charset, &ds->charset8));
  if (ds->initstmt   ) printf("SSL key:             %s\n", ds_get_utf8attr(ds->sslkey, &ds->sslkey8));
  if (ds->initstmt   ) printf("SSL cert:            %s\n", ds_get_utf8attr(ds->sslcert, &ds->sslcert8));
  if (ds->initstmt   ) printf("SSL CA:              %s\n", ds_get_utf8attr(ds->sslca, &ds->sslca8));
  if (ds->initstmt   ) printf("SSL CA path:         %s\n", ds_get_utf8attr(ds->sslcapath, &ds->sslcapath8));
  if (ds->initstmt   ) printf("SSL cipher:          %s\n", ds_get_utf8attr(ds->sslcipher, &ds->sslcipher8));
  if (ds->sslverify  ) printf("Verify SSL cert      yes\n");
  if (ds->port       ) printf("Port:                %d\n", ds->port);

  return 0;
}


/*
 * Handler for "list data sources" command (-s -l)
 */
int list_datasources()
{
  SQLHANDLE env;
  SQLRETURN rc;
  SQLUSMALLINT dir= 0; /* SQLDataSources fetch direction */
  SQLCHAR name[256];
  SQLCHAR description[256];

  /* determine 'direction' to pass to SQLDataSources */
  switch (scope)
  {
  case ODBC_BOTH_DSN:
    dir= SQL_FETCH_FIRST;
    break;
  case ODBC_USER_DSN:
    dir= SQL_FETCH_FIRST_USER;
    break;
  case ODBC_SYSTEM_DSN:
    dir= SQL_FETCH_FIRST_SYSTEM;
    break;
  }

  /* allocate handle and set ODBC API v3 */
  if ((rc= SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE,
                          &env)) != SQL_SUCCESS)
  {
    fprintf(stderr, "[ERROR] Failed to allocate env handle\n");
    return 1;
  }

  if ((rc= SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3,
                         SQL_IS_INTEGER)) != SQL_SUCCESS)
  {
    print_odbc_error(env, SQL_HANDLE_ENV);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return 1;
  }

  /* retrieve and print data source */
  while ((rc= SQLDataSources(env, dir, name, 256, NULL, description,
                             256, NULL)) == SQL_SUCCESS)
  {
    printf("%-20s - %s\n", name, description);
    dir= SQL_FETCH_NEXT;
  }

  if (rc != SQL_NO_DATA)
  {
    print_odbc_error(env, SQL_HANDLE_ENV);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return 1;
  }

  SQLFreeHandle(SQL_HANDLE_ENV, env);

  return 0;
}


/*
 * Handler for "add data source" command (-s -a -n drivername -t attrs)
 */
int add_datasource(DataSource *ds, const SQLWCHAR *attrs)
{
  /* read datasource object from attributes */
  if (ds_from_kvpair(ds, attrs, ';'))
  {
    fprintf(stderr,
            "[ERROR] Could not parse key-value pair attribute string\n");
    return 1;
  }

  /* validate */
  if (!ds->driver)
  {
    fprintf(stderr, "[ERROR] Driver must be specified for a data source\n");
    return 1;
  }

  /* Add it */
  if (ds_add(ds))
  {
    print_installer_error();
    fprintf(stderr,
            "[ERROR] Data source entry failed, remove or try again\n");
    return 1;
  }

  printf("Success\n");
  return 0;
}


/*
 * Handler for "remove data source" command (-s -r -n drivername)
 */
int remove_datasource(DataSource *ds)
{
  /*
     First check that it exists, because SQLRemoveDSNFromIni
     returns true, even if it doesn't.
   */
  if (ds_exists(ds->name))
  {
    fprintf(stderr, "[ERROR] Data source doesn't exist\n");
    return 1;
  }

  if (SQLRemoveDSNFromIniW(ds->name) != TRUE)
  {
    print_installer_error();
    return 1;
  }

  printf("Success\n");

  return 0;
}


/*
 * Handle driver actions. We setup a data source object to be used
 * for all actions. Config/scope set+restore is wrapped around the
 * action.
 */
int handle_datasource_action()
{
  int rc= 0;
  UWORD config_mode= config_set(scope);
  DataSource *ds= ds_new();

  /* check name is given if needed */
  switch (action)
  {
  case ACTION_ADD:
  case ACTION_REMOVE:
    if (!name)
    {
      fprintf(stderr, "[ERROR] Name missing to add/remove data source\n");
      rc= 1;
      goto end;
    }
  }

  /* set name if given */
  if (name)
    ds_set_strattr(&ds->name, wname);

  /* perform given action */
  switch (action)
  {
  case ACTION_LIST:
    if (name)
      rc= list_datasource_details(ds);
    else
      rc= list_datasources();
    break;
  case ACTION_ADD:
    if (scope == ODBC_BOTH_DSN)
    {
      fprintf(stderr, "[ERROR] Adding data source must be either "
                      "user or system scope (not both)\n");
      rc= 1;
    }
    else if (!attrstr)
    {
      fprintf(stderr, "[ERROR] Attribute string missing to add data source\n");
      rc= 1;
    }
    else
    {
      rc= add_datasource(ds, wattrs);
    }
    break;
  case ACTION_REMOVE:
    rc= remove_datasource(ds);
    break;
  }

end:
  ds_delete(ds);
  config_set(config_mode);
  return rc;
}


/*
 * Entry point, parse args and do first set of validation.
 */
int main(int argc, char **argv)
{
  char *arg;
  int i;
  SQLINTEGER convlen;

  /* minimum number of args to do anything useful */
  if (argc < 3)
  {
    fprintf(stderr, "[ERROR] Not enough arguments given\n");
    main_usage();
    return 1;
  }

  /* parse args */
  for(i= 1; i < argc; ++i)
  {
    arg= argv[i];
    if (*arg == '-')
      arg++;

    /* we should be left with a single character option (except scope)
     * all strings are skipped by the respective option below */
    if (*arg != OPT_SCOPE && *(arg + 1))
    {
      fprintf(stderr, "[ERROR] Invalid command line option: %s\n", arg);
      return 1;
    }

    switch (*arg)
    {
    case OBJ_DRIVER:
    case OBJ_DATASOURCE:
      /* make sure we haven't already assigned the object */
      if (obj)
      {
        object_usage();
        return 1;
      }
      obj= *arg;
      break;
    case ACTION_LIST:
    case ACTION_ADD:
    case ACTION_REMOVE:
      if (action)
      {
        action_usage();
        return 1;
      }
      action= *arg;
      break;
    case OPT_NAME:
      if (i + 1 == argc || *argv[i + 1] == '-')
      {
        fprintf(stderr, "[ERROR] Missing name\n");
        return 1;
      }
      name= argv[++i];
      break;
    case OPT_ATTR:
      if (i + 1 == argc || *argv[i + 1] == '-')
      {
        fprintf(stderr, "[ERROR] Missing attribute string\n");
        return 1;
      }
      attrstr= argv[++i];
      break;
    case OPT_SCOPE:
      /* convert to integer */
      scope= *(++arg) - '0';
      if (scope > 2 || *(arg + 1) /* another char exists */)
      {
        fprintf(stderr, "[ERROR] Invalid scope: %s\n", arg);
        return 1;
      }
      break;
    case 'h':
      /* print usage if -h is given anywhere */
      main_usage();
      return 1;
    default:
      fprintf(stderr, "[ERROR] Invalid command line option: %s\n", arg);
      return 1;
    }
  }

  if (!action)
  {
    action_usage();
    return 1;
  }

  /* init libmysqlclient */
  my_init();
  utf8_charset_info= get_charset_by_csname("utf8", MYF(MY_CS_PRIMARY),
                                           MYF(0));

  /* convert to SQLWCHAR for installer API */
  convlen= SQL_NTS;
  if (name && !(wname= sqlchar_as_sqlwchar(default_charset_info,
                                           (SQLCHAR *)name, &convlen, NULL)))
  {
    fprintf(stderr, "[ERROR] Name is invalid\n");
    return 1;
  }

  convlen= SQL_NTS;
  if (attrstr && !(wattrs= sqlchar_as_sqlwchar(default_charset_info,
                                               (SQLCHAR *)attrstr, &convlen,
                                               NULL)))
  {
    fprintf(stderr, "[ERROR] Attribute string is invalid\n");
    return 1;
  }

  /* handle appropriate action */
  switch (obj)
  {
  case OBJ_DRIVER:
    return handle_driver_action();
  case OBJ_DATASOURCE:
    return handle_datasource_action();
  default:
    object_usage();
    return 1;
  }
}

