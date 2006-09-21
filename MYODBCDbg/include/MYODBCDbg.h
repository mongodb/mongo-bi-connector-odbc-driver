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

/* #include "../../MYODBC_MYSQL.h" */
#include "../../MYODBC_CONF.h"
#include "../../MYODBC_ODBC.h"

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
    \brief  Initializes debug output.
                
            Call this before using any of the MYODBCDbg functions.
            
    \sa     MYODBCDbgPrint
            MYODBCDbgSetFile
*/
#ifdef DBUG_OFF
#define MYODBCDbgInit
#else
#define MYODBCDbgInit \
{ \
    char *psz = getenv( "MYODBC_LOG" ); \
    if ( psz && *psz ) \
        MYODBCDbgSetFile( psz ) \
}
#endif

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

/*! 
    \brief  Sets debug control information.
                
            Sets debug control information including filename for output.

    \note   This has not effect if DBUG_OFF is set.

    \param  pszControl  This is a "const char *" which indicates the control
                        information. Example;

                        "d:t:S:O,/tmp/myodbc.log"
    
    \return void
    
    \sa     MYODBCDbgInit
            MYODBCDbgFini
            MYODBCDbgPrint
*/
#define MYODBCDbgSetFile(A) \
{ \
    DBUG_PUSH(A); \
    DBUG_PROCESS( "MYODBC" ); \
    MYODBCDbgPrint( "start", ( "Driver name: Connector/ODBC  Version: %s", VERSION ) ); \
}

/*! 
    \brief  Inits debug for a function.
                
            This declares some working vars for dbg interface and logs that
            we have entered the function. 

    \note   This has not effect if DBUG_OFF is set.

    \param  pszMsg  This is a "const char *" which can be any message.
    
    \return void
    
    \sa     MYODBCDbgInit
            MYODBCDbgFini
            MYODBCDbgPrint
            MYODBCDbgReturn
*/
#define MYODBCDbgEnter DBUG_ENTER

/*! 
    \brief  Prints useful debug information.
                
            This processes the given keyword & argslist and prints the result according 
            to the debug control information.

    \note   This has not effect if DBUG_OFF is set.

    \param  keyword
    \param  arglist 
    
    \return void
    
    \sa     MYODBCDbgInit
            MYODBCDbgFini
*/
#define MYODBCDbgPrint DBUG_PRINT

/*! 
    \brief  Stops the debug.

            Stops the debug and frees resources allocated in MYODBCDbgInit.

    \note   This has not effect if DBUG_OFF is set.

    \return void
    
    \sa     MYODBCDbgInit
            MYODBCDbgFini
*/
#define MYODBCDbgFini {}

/*! 
    \brief  Returns from a function.
                
            Prints that we are leaving a function and what the retval is.

    \note   This has not effect if DBUG_OFF is set.

    \param  SQLRETURN
    
    \return SQLRETURN
    
    \sa     MYODBCDbgInit
            MYODBCDbgFini
            MYODBCDbgEnter
            MYODBCDbgReturn2
            MYODBCDbgReturn3
*/
#ifdef DBUG_OFF
#define MYODBCDbgReturn(A) return (A);
#else
#define MYODBCDbgReturn(A) \
{ \
  SQLRETURN rc= (A); \
  MYODBCDbgPrint("exit", ("%s", MYODBCDbgReturnString( rc )) ); \
  DBUG_RETURN( rc ); \
}
#endif

const char *MYODBCDbgGetFileDefault();

/*! 
    \brief  Returns from a function.
                
            Prints that we are leaving a function and what the retval is.

    \note   This has not effect if DBUG_OFF is set.

    \param  SQLRETURN
    
    \return SQLRETURN
    
    \sa     MYODBCDbgInit
            MYODBCDbgFini
            MYODBCDbgEnter
            MYODBCDbgReturn
            MYODBCDbgReturn3
*/
#define MYODBCDbgReturn2 DBUG_RETURN

/*! 
    \brief  Returns from a function.
                
            Prints that we are leaving a function.

    \note   This has not effect if DBUG_OFF is set.

    \sa     MYODBCDbgInit
            MYODBCDbgFini
            MYODBCDbgEnter
            MYODBCDbgReturn2
            MYODBCDbgReturn
*/
#define MYODBCDbgReturn3 DBUG_VOID_RETURN

#endif

