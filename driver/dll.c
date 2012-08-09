/*
  Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.

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
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file  dll.c
  @brief Library initialization functions.
*/

#include "driver.h"
#include <locale.h>

char *default_locale, *decimal_point, *thousands_sep;
uint decimal_point_length,thousands_sep_length;
static my_bool myodbc_inited=0;


/*
  Sigpipe handler
*/

#if !defined(__WIN__) && defined(SIGPIPE)

#include <signal.h>

static sig_handler
myodbc_pipe_sig_handler(int sig __attribute__((unused)))
{
  /* Do nothing */
}

#endif

/*
  @type    : myodbc3 internal
  @purpose : initializations
*/

void myodbc_init(void)
{
#if !defined(__WIN__) && defined(SIGPIPE)
   /*
     sigaction will block other signals from coming when handler is working
   */
   struct sigaction action;
   action.sa_handler = myodbc_pipe_sig_handler;
   sigemptyset(&action.sa_mask);
   action.sa_flags = 0;
   sigaction(SIGPIPE, &action, NULL);
#endif

  if (myodbc_inited++)
    return;
  my_init();
  {
    struct lconv *tmp;
    init_getfunctions();
    default_locale=my_strdup(setlocale(LC_NUMERIC,NullS),MYF(0));
    setlocale(LC_NUMERIC,"");
    tmp=localeconv();
    decimal_point=my_strdup(tmp->decimal_point,MYF(0));
    decimal_point_length=strlen(decimal_point);
    thousands_sep=my_strdup(tmp->thousands_sep,MYF(0));
    thousands_sep_length=strlen(thousands_sep);
    setlocale(LC_NUMERIC,default_locale);

    utf8_charset_info= get_charset_by_csname("utf8", MYF(MY_CS_PRIMARY),
                                             MYF(0));
  }
}


/*
  @type    : myodbc3 internal
  @purpose : clean all resources while unloading..
*/
void myodbc_end()
{
  if (!--myodbc_inited)
  {
    x_free(decimal_point);
    x_free(default_locale);
    x_free(thousands_sep);

    /* my_thread_end_wait_time was added in 5.1.14 and 5.0.32 */
#if !defined(NONTHREADSAFE) && \
    (MYSQL_VERSION_ID >= 50114 || \
     (MYSQL_VERSION_ID >= 50032 && MYSQL_VERSION_ID < 50100))
    /*
       This eliminates the delay when my_end() is called and other threads
       have been initialized but not ended.
    */
    my_thread_end_wait_time= 0;
#endif

#ifdef MY_DONT_FREE_DBUG
    /*
       Function my_end() was changed to deallocate DBUG memory by default,
       a flag MY_DONT_FREE_DBUG was added to disable this new behaviour
    */
    my_end(MY_DONT_FREE_DBUG);
#else
    my_end(0);
#endif
  }
}


/*
  @type    : myodbc3 internal
  @purpose : main entry point
*/

#ifdef _WIN32
static int inited= 0;
int APIENTRY LibMain(HANDLE hInst, DWORD ul_reason_being_called,
		     LPVOID lpReserved)
{
  switch (ul_reason_being_called) {
  case DLL_PROCESS_ATTACH:  /* case of libentry call in win 3.x */
    if (!inited++)
      myodbc_init();
    break;
  case DLL_PROCESS_DETACH:  /* case of wep call in win 3.x */
    if (!--inited)
      myodbc_end();
    break;

  /*
     We don't explicitly call my_thread_init() to avoid initialization in
     threads that may not even make ODBC calls. my_thread_init() will be
     called implicitly when mysys calls are made from the thread.
  */
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
#ifdef THREAD
    my_thread_end();
#endif
    break;

  default:
    break;
  }

  return TRUE;

  UNREFERENCED_PARAMETER(lpReserved);
}


/*
  @type    : myodbc3 internal
  @purpose : entry point for the DLL
*/

int __stdcall DllMain(HANDLE hInst,DWORD ul_reason_being_called,
		      LPVOID lpReserved)
{
  return LibMain(hInst,ul_reason_being_called,lpReserved);
}

#elif defined(__WIN__)

/***************************************************************************
  This routine is called by LIBSTART.ASM at module load time.  All it
  does in this sample is remember the DLL module handle.  The module
  handle is needed if you want to do things like load stuff from the
  resource file (for instance string resources).
***************************************************************************/

int _export FAR PASCAL libmain(HANDLE hModule,short cbHeapSize,
	     SQLCHAR FAR *lszCmdLine)
{
  myodbc_init();
  return TRUE;
}

#endif /* __WIN__ */

#ifdef _WIN32
void __declspec(dllexport) FAR PASCAL LoadByOrdinal(void);
/* Entry point to cause DM to load using ordinals */
void __declspec(dllexport) FAR PASCAL LoadByOrdinal(void)
{}
#endif
