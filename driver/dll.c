/*
  Copyright (C) 1995-2007 MySQL AB

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
#ifdef DONT_REMEMBER_SIGNAL
  (void) signal(SIGPIPE,myodbc_pipe_sig_handler);
#endif
}

#endif

/*
  @type    : myodbc3 internal
  @purpose : initializations
*/

void myodbc_init(void)
{
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
  /*
    If we are not using threads, we may get an SIGPIPE signal when a client
    aborts.  We disable this signal to avoid problems.
  */
#if !defined(__WIN__) && defined(SIGPIPE)
  signal(SIGPIPE,myodbc_pipe_sig_handler);
#endif
}

/*
  @type    : myodbc3 internal
  @purpose : clean all resources while unloading..
*/

void myodbc_end()
{
 if (!--myodbc_inited)
 {
   my_free(decimal_point,MYF(0));
   my_free(default_locale,MYF(0));
   my_free(thousands_sep,MYF(0));
#ifdef NOT_YET_USED
   mysql_server_end();
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
static int inited=0,threads=0;
static DWORD main_thread;
HINSTANCE NEAR s_hModule; /* Saved module handle */
int APIENTRY LibMain(HANDLE hInst,DWORD ul_reason_being_called,
		     LPVOID lpReserved)
{
  switch (ul_reason_being_called) {
  case DLL_PROCESS_ATTACH:  /* case of libentry call in win 3.x */
  if (!inited++)
  {
    s_hModule=hInst;
    myodbc_init();
    main_thread=GetCurrentThreadId();
  }
  break;
  case DLL_THREAD_ATTACH:
    threads++;
#ifdef THREAD
    my_thread_init();
#endif
    break;
  case DLL_PROCESS_DETACH:  /* case of wep call in win 3.x */
    if (!--inited)
      myodbc_end();
    break;
  case DLL_THREAD_DETACH:
#ifdef THREAD
    /* Main thread will free by my_end() */
    threads--;
    if (main_thread != GetCurrentThreadId())
      my_thread_end();
#else
    --threads;
#endif
    break;
  default:
    break;
  } /* switch */

  return TRUE;

  UNREFERENCED_PARAMETER(lpReserved);
} /* LibMain */

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
  s_hModule = hModule;
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
