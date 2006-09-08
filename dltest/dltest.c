#include <stdio.h>
#include <stdlib.h>

#ifdef _UNIX_
    #include <ltdl.h>
#else
    #include <windows.h>
#endif

char *szSyntax =
"\n" \
"**********************************************\n" \
"* dltest                                     *\n" \
"**********************************************\n" \
"* Syntax                                     *\n" \
"*                                            *\n" \
"*      dltest libName Symbol                 *\n" \
"*                                            *\n" \
"* libName                                    *\n" \
"*                                            *\n" \
"*      Full path + file name of share to test*\n" \
"*                                            *\n" \
"* Symbol                                     *\n" \
"*                                            *\n" \
"*      ie a function name in the share       *\n" \
"*                                            *\n" \
"* Notes                                      *\n" \
"*                                            *\n" \
"*      This can be placed into a makefile    *\n" \
"*      to throw an error if test fails.      *\n" \
"*                                            *\n" \
"*      If this segfaults you probably have an*\n" \
"*      unresolved symbol in the lib. This is *\n" \
"*      not caught since dltest started using *\n" \
"*      libtool. Linux users can refer to the *\n" \
"*      man page for dlopen to create a       *\n" \
"*      better test.                          *\n" \
"*                                            *\n" \
"*                                            *\n" \
"* Examples                                   *\n" \
"*                                            *\n" \
"*      dltest /usr/lib/libMy.so MyFunc       *\n" \
"*                                            *\n" \
"**********************************************\n\n";

int main( int argc, char *argv[] )
{
#ifdef WIN32
    HMODULE         hModule     = NULL;
#else
    void *          hModule     = NULL;
#endif
    void    (*pFunc)();

    if ( argc < 2 )
    {
        printf( szSyntax );
        exit( 1 );
    }

    /*
     * initialize libtool
     */

#ifdef _UNIX_
    if ( lt_dlinit() )
    {
        printf( "[%s][%d] ERROR: Failed to lt_dlinit()\n", __FILE__, __LINE__ );
        exit( 1 );
    }

    hModule = lt_dlopen( argv[1] );
    if ( !hModule )
    {
        printf( "[%s][%d] ERROR dlopen(): %s\n", __FILE__, __LINE__, lt_dlerror() );
        exit( 1 );
    }
    printf( "[%s][%d] SUCCESS: Loaded %s\n", __FILE__, __LINE__, argv[1] );
    if ( argc > 2 )
    {
        pFunc = (void (*)()) lt_dlsym( hModule, argv[2] );
/* PAH - lt_dlerror() is not a good indicator of success    */
/*		if ( (pError = lt_dlerror()) != NULL )              */
        if ( !pFunc )
        {
            const char *pError;

            if ( (pError = lt_dlerror()) != NULL )
                printf( "[%s][%d] ERROR: %s\n Could not find %s\n", __FILE__, __LINE__, pError, argv[2] );
            else
                printf( "[%s][%d] ERROR: Could not find %s\n", __FILE__, __LINE__, argv[2] );
            exit( 1 );
        }
        printf( "[%s][%d] SUCCESS: Found %s\n", __FILE__, __LINE__, argv[2] );
    }
    lt_dlclose( hModule );
#else
    if ( !(hModule = LoadLibrary( (LPCSTR)argv[1] )) )
    {
        LPVOID pszMsg;

        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       GetLastError(),
                       MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                       (LPTSTR) &pszMsg,
                       0, 
                       NULL );
        printf( "[%s][%d] ERROR LoadLibrary(): %s\n", __FILE__, __LINE__, pszMsg );
        LocalFree( pszMsg );
        exit( 1 );
    }

    printf( "[%s][%d] SUCCESS: Loaded %s\n", __FILE__, __LINE__, argv[1] );
    if ( argc > 2 )
    {
        pFunc = (void (*)()) GetProcAddress( hModule, argv[2] );
        if ( !pFunc )
        {
            LPVOID pszMsg;
    
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           GetLastError(),
                           MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                           (LPTSTR) &pszMsg,
                           0, 
                           NULL );
            printf( "[%s][%d] ERROR: Could not find %s. %s\n", __FILE__, __LINE__, argv[2], pszMsg );
            LocalFree( pszMsg );
            FreeLibrary( hModule );
            exit( 1 );
        }
        printf( "[%s][%d] SUCCESS: Found %s\n", __FILE__, __LINE__, argv[2] );
    }
    FreeLibrary( hModule );
#endif

    return( 0 );
}

