/*! 
    \file     MYODBCDbg.h
    \author   Peter Harvey <pharvey@mysql.com>
              Copyright (C) MySQL AB 2004-2005, Released under GPL.
    \version  Connector/ODBC v3
    \date     2005          
    \brief    Code to provide debug information.
    
    \license  Copyright (C) 2000-2005 MySQL AB

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
              Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef MYODBC_DBG_H
#define MYODBC_DBG_H

#include <stdio.h>

/* #include "../../MYODBC_MYSQL.h" */

#include "../../MYODBC_CONF.h"
#include "../../MYODBC_ODBC.h"

extern	int     MYODBCDbgOn;
extern	FILE *  MYODBCDbgFile;
extern  int     MYODBCDbgNest;

#ifdef MYODBC_DBG

#ifndef USE_GNU_FUNC_MACRO
#  ifdef USE_C99_FUNC_MACRO
#    define __FUNCTION__ __func__
#  else
#    define __FUNCTION__ ""
#  endif
#endif

#define MYODBCDbgInit \
{ \
    char *pszMyODBCLog = getenv( "MYODBC_LOG" ); \
    MYODBCDbgOn = 0; \
    MYODBCDbgFile = NULL; \
    MYODBCDbgNest = 0; \
    if ( pszMyODBCLog && *pszMyODBCLog ) \
    { \
        if ( strcmp( pszMyODBCLog, "off" ) == 0 ) \
        { \
        } \
        else if ( strcmp( pszMyODBCLog, "stdout" ) == 0 || strcmp( pszMyODBCLog, "stderr" ) == 0 ) \
        { \
            MYODBCDbgSetFile( pszMyODBCLog ); \
        } \
        else \
        { \
            MYODBCDbgSetFile( pszMyODBCLog ); \
            if ( !MYODBCDbgFile ) \
            { \
                MYODBCDbgSetFile( "stderr" ); \
            } \
        } \
    } \
    if ( MYODBCDbgFile ) \
        MYODBCDbgSetOn; \
}

#define MYODBCDbgFini \
{ \
    if ( MYODBCDbgFile ) \
    { \
        if ( MYODBCDbgFile != stdout && MYODBCDbgFile != stderr ) \
            fclose( MYODBCDbgFile ); \
    } \
    MYODBCDbgOn = 0; \
    MYODBCDbgFile = NULL; \
    MYODBCDbgNest = 0; \
}

#define MYODBCDbgEnter \
{ \
    int nNest = 0; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[ENTER][%s][%s][%d]\n", __FUNCTION__, __FILE__, __LINE__ ); \
    } \
    MYODBCDbgNest++; \
}

#define MYODBCDbgReturn( A ) \
{ \
    int nNest = 0; \
    MYODBCDbgNest--; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[RETURN][%s][%s][%d]\n", __FUNCTION__, __FILE__, __LINE__ ); \
    } \
    return ( A ); \
}

#define MYODBCDbgReturnReturn( A ) \
{ \
    int nNest = 0; \
    SQLRETURN nReturn_ = A; \
    MYODBCDbgNest--; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[RETURN][%s][%s][%d] %s\n", __FUNCTION__, __FILE__, __LINE__, MYODBCDbgReturnString( nReturn_ ) ); \
    } \
    return ( nReturn_ ); \
}

#define MYODBCDbgReturnVoid \
{ \
    int nNest = 0; \
    MYODBCDbgNest--; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[RETURN][%s][%s][%d]\n", __FUNCTION__, __FILE__, __LINE__ ); \
    } \
}

#define MYODBCDbgInfo( A, B ) \
{ \
    int nNest = 0; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[INFO][%s][%s][%d]" A "\n", __FUNCTION__, __FILE__, __LINE__, B ); \
    } \
}

#define MYODBCDbgWarning( A, B ) \
{ \
    int nNest = 0; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[WARNING][%s][%s][%d]" A "\n", __FUNCTION__, __FILE__, __LINE__, B ); \
    } \
}

#define MYODBCDbgError( A, B ) \
{ \
    int nNest = 0; \
    if ( MYODBCDbgOn && MYODBCDbgFile ) \
    { \
        for ( nNest = 0; nNest < MYODBCDbgNest; nNest++ ) \
        { \
            fprintf( MYODBCDbgFile, "|   " ); \
        } \
        fprintf( MYODBCDbgFile, "[ERROR][%s][%s][%d]" A "\n", __FUNCTION__, __FILE__, __LINE__, B ); \
    } \
}

#define MYODBCDbgSetOn \
{ \
    MYODBCDbgOn = 1; \
}

#define MYODBCDbgSetOff \
{ \
    MYODBCDbgOn = 0; \
}

#define MYODBCDbgSetFile( A ) \
{ \
    if ( MYODBCDbgFile ) \
    { \
        if ( MYODBCDbgFile != stdout && MYODBCDbgFile != stderr ) \
            fclose( MYODBCDbgFile ); \
    } \
    if ( strcmp( A, "stdout" ) == 0 ) \
        MYODBCDbgFile = stdout; \
    else if ( strcmp( A, "stderr" ) == 0 ) \
        MYODBCDbgFile = stderr; \
    else \
    { \
        MYODBCDbgFile = fopen( A, "aw+" ); \
    } \
}

#else
    #define MYODBCDbgInit
    #define MYODBCDbgFini
    #define MYODBCDbgEnter
    #define MYODBCDbgReturn( A ) return ( A )
    #define MYODBCDbgReturnReturn( A ) return ( A )
    #define MYODBCDbgReturnVoid return
    #define MYODBCDbgInfo( A, B )
    #define MYODBCDbgWarning( A, B )
    #define MYODBCDbgError( A, B )
    #define MYODBCDbgSetOn
    #define MYODBCDbgSetOff
    #define MYODBCDbgSetFile( A )
#endif

/*! 
    \brief  Returns a string version of a connection attribute.
                
            Returns a string version of a connection attribute. This is great for
            returning more readable debug output.
            
    \param  nAttribute  An connection attribute.
    
    \return A string constant describing nAttribute.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgConnectAttrString( SQLINTEGER nAttribute );

/*! 
    \brief  Returns a string version of a connection option.
                
            Returns a string version of a connection option (or statement attribute).
            This is great for returning more readable debug output.
            
    \param  nOption  A connection option.
    
    \return A string constant describing nOption.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgConnectOptionString( SQLUSMALLINT nOption );

/*! 
    \brief  Returns a string version of a diag field identifier.
                
            Returns a string version of a diag field identifier. This is great for
            returning more readable debug output.
            
    \param  nDiagField  A diag field identifier.
    
    \return A string constant describing nDiagField.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgDiagFieldString( SQLSMALLINT nDiagField );

/*! 
    \brief  Returns a string version of a environment attribute.
                
            Returns a string version of a environment attribute. This is great for
            returning more readable debug output.
            
    \param  nAttribute  An environment attribute.
    
    \return A string constant describing nAttribute.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgEnvAttrString( SQLINTEGER nAttribute );


/*! 
    \brief  Returns a string version of a function id.
                
            Returns a string version of a function id. This is great for
            returning more readable debug output.
            
    \param  nFunction  A function ID or SQL_API_ALL_FUNCTIONS, or 
                       SQL_API_ODBC3_ALL_FUNCTIONS.
    
    \return A string of the function ID.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgFunctionsString( SQLUSMALLINT nFunction );

/*! 
    \brief  Returns a string version of a handle type identifier.
                
            Returns a string version of a handle type identifier. This is great for
            returning more readable debug output.
            
    \param  nHandleType A handle type identifier.
    
    \return A string constant describing nHandleType.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgHandleTypeString( SQLSMALLINT nHandleType );

/*! 
    \brief  Returns a string version of a info type identifier.
                
            Returns a string version of a info type identifier. This is great for
            returning more readable debug output.
            
    \param  nInfoType A info type identifier.
    
    \return A string constant describing nInfoType.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgInfoTypeString( SQLUSMALLINT nInfoType );

/*! 
    \brief  Returns a string version of the position type as found in SQLSetPos.
                
            Returns a string version of position type. This is great for
            returning more readable debug output.
            
    \param  nType Position type as used by SQLSetPos.
    
    \return A string constant describing nType.
    
    \sa     MYODBCDbgReturn
*/
const char *MYODBCDbgPosTypeString( SQLUSMALLINT nType );

/*! 
    \brief  Returns a string version of SQLRETURN.
                
            Returns a string version of SQLRETURN. This is great for
            returning more readable debug output.
            
    \param  ret A SQLRETURN value.
    
    \return A string constant describing ret.
    
    \sa     MYODBCDbgReturn
*/
const char *MYODBCDbgReturnString( SQLRETURN nReturn );

/*! 
    \brief  Returns a string version of a statement attribute.
                
            Returns a string version of a statement attribute. This is great for
            returning more readable debug output.
            
    \param  nAttribute  An statement attribute.
    
    \return A string constant describing nAttribute.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgStmtAttrString( SQLINTEGER nAttribute );

/*! 
    \brief  Returns a string version of a statement option.
                
            Returns a string version of a statement option. This is great for
            returning more readable debug output.
            
    \param  nOption  An statement option.
    
    \return A string constant describing nOption.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgStmtOptionString( SQLUSMALLINT nOption );

/*! 
    \brief  Returns a string version of a statement type as used by SQLFreeStmt.
                
            Returns a string version of a statement type. This is great for
            returning more readable debug output.
            
    \param  nType   A statement type as used for SQLFreeStmt.
    
    \return A string constant describing nType.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgStmtTypeString( SQLUSMALLINT nType );

/*! 
    \brief  Returns a string version of a transaction type as used by SQLEndTran.
                
            Returns a string version of a transaction type. This is great for
            returning more readable debug output.
            
    \param  nType   A transaction type as used for SQLEndTran.
    
    \return A string constant describing nType.
    
    \sa     MYODBCDbgPrint
*/
const char *MYODBCDbgTransactionTypeString( SQLSMALLINT nType );


#endif

