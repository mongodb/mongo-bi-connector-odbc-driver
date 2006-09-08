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

#ifndef ODBCCONNECTION_H
#define ODBCCONNECTION_H

//
#include "MYODBCObject.h"
#include "MYODBCStatement.h"
#include "MYODBCMessage.h"

class MYODBCEnvironment;

// wraps SQLHDBC

class MYODBCConnection : public MYODBCObject
{
public:
    MYODBCConnection( MYODBCEnvironment *penvironment );
    virtual ~MYODBCConnection();

    // SETTERS
    // attributes
    virtual SQLRETURN setSQL_ATTR_ACCESS_MODE( SQLUINTEGER nSQL_ATTR_ACCESS_MODE );
    virtual SQLRETURN setSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER nSQL_ATTR_ASYNC_ENABLE ); 
    virtual SQLRETURN setSQL_ATTR_CONNECTION_TIMEOUT( SQLUINTEGER nSQL_ATTR_CONNECTION_TIMEOUT );
    virtual SQLRETURN setSQL_ATTR_CURRENT_CATALOG( SQLCHAR *pszSQL_ATTR_CURRENT_CATALOG );
    virtual SQLRETURN setSQL_ATTR_LOGIN_TIMEOUT( SQLUINTEGER nSQL_ATTR_LOGIN_TIMEOUT );
    virtual SQLRETURN setSQL_ATTR_PACKET_SIZE( SQLUINTEGER nSQL_ATTR_PACKET_SIZE );
    virtual SQLRETURN setSQL_ATTR_TRACE( SQLUINTEGER nSQL_ATTR_TRACE );
    virtual SQLRETURN setSQL_ATTR_TRACEFILE( SQLCHAR *pszSQL_ATTR_TRACEFILE );

    // GETTERS
    // attributes
    virtual SQLRETURN getSQL_ATTR_ACCESS_MODE( SQLUINTEGER *pnSQL_ATTR_ACCESS_MODE );
    virtual SQLRETURN getSQL_ATTR_ASYNC_ENABLE( SQLUINTEGER *pnSQL_ATTR_ASYNC_ENABLE );
    virtual SQLRETURN getSQL_ATTR_CONNECTION_TIMEOUT( SQLUINTEGER *pnSQL_ATTR_CONNECTION_TIMEOUT );
    virtual SQLRETURN getSQL_ATTR_CURRENT_CATALOG( SQLCHAR *pszSQL_ATTR_CURRENT_CATALOG, SQLINTEGER nLength, SQLINTEGER *pnReturnLength );
    virtual SQLRETURN getSQL_ATTR_LOGIN_TIMEOUT( SQLUINTEGER *pnSQL_ATTR_LOGIN_TIMEOUT );
    virtual SQLRETURN getSQL_ATTR_PACKET_SIZE( SQLUINTEGER *pnSQL_ATTR_PACKET_SIZE );
    virtual SQLRETURN getSQL_ATTR_TRACE( SQLUINTEGER *pnSQL_ATTR_TRACE );
    virtual SQLRETURN getSQL_ATTR_TRACEFILE( SQLCHAR *pszSQL_ATTR_TRACEFILE, SQLINTEGER nLength, SQLINTEGER *pnReturnLength );


//    virtual SQLRETURN getSQL_ASYNC_MODE( SQLUINTEGER *pn );

    virtual bool getIsConnected();
    virtual SQLRETURN getInfo( SQLUSMALLINT nInfoType, SQLPOINTER pInfoValue, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLength );
    virtual MYODBCEnvironment *getEnvironment() { return penvironment; }
    virtual SQLHDBC getDbc() { return hDbc; }
    virtual const char *getZClassName() { return "MYODBCConnection"; }
    virtual MYODBCMessage *getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState = 0, char *pszMessage = 0, SQLINTEGER nNativeCode = 0 );
    virtual MYODBCObjectList *getMessages() { return &objectlistMessages; }
    
    // DO'rs
    virtual SQLRETURN doAlloc();
    virtual SQLRETURN doFree( bool bForce = false );
    virtual SQLRETURN doConnect( SQLCHAR *pszServerName = NULL, SQLSMALLINT nLength1 = SQL_NTS, SQLCHAR *pszUserName = NULL, SQLSMALLINT nLength2 = SQL_NTS, SQLCHAR *pszAuthentication = NULL, SQLSMALLINT nLength3 = SQL_NTS );
    virtual SQLRETURN doDriverConnect( SQLHWND hWnd, SQLCHAR *pszIn, SQLSMALLINT nLengthIn, SQLCHAR *pszOut, SQLSMALLINT nLengthOut, SQLSMALLINT *pnLengthOut, SQLUSMALLINT nPrompt );
    virtual SQLRETURN doBrowseConnect( SQLCHAR *szInConnectionString, SQLSMALLINT nStringLength1, SQLCHAR *szOutConnectionString, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLength2Ptr );
    virtual SQLRETURN doDisconnect();
    virtual void doErrors( MYODBCMessage::MessageTypes nType );

protected:
    MYODBCEnvironment *       penvironment;
    SQLHDBC	                hDbc;
    bool                    bConnected;
    MYODBCObjectList          objectlistMessages;

    // Attributes
    // We need these because we want the user to be able to set/get
    // the values even when we do not have a SQLHDBC.

    // Attribute Flags
    // These flags indicate whether or not we should set the Attribute Value. They
    // are set, and remain set, whenever the Value is set.
    bool bSQL_ATTR_ACCESS_MODE;
    bool bSQL_ATTR_ASYNC_ENABLE; 
    bool bSQL_ATTR_CONNECTION_TIMEOUT;
    bool bSQL_ATTR_CURRENT_CATALOG;
    bool bSQL_ATTR_LOGIN_TIMEOUT;
    bool bSQL_ATTR_PACKET_SIZE;
    bool bSQL_ATTR_TRACE;
    bool bSQL_ATTR_TRACEFILE;

    // Attribute Values
    // These are the desired attribute values. Only used if Attribute Flag is True. 
    SQLUINTEGER nSQL_ATTR_ACCESS_MODE;
    SQLUINTEGER nSQL_ATTR_ASYNC_ENABLE; 
    SQLUINTEGER nSQL_ATTR_CONNECTION_TIMEOUT;
    SQLCHAR *   pszSQL_ATTR_CURRENT_CATALOG;
    SQLUINTEGER nSQL_ATTR_LOGIN_TIMEOUT;
    SQLUINTEGER nSQL_ATTR_PACKET_SIZE;
    SQLUINTEGER nSQL_ATTR_TRACE;
    SQLCHAR *   pszSQL_ATTR_TRACEFILE;

    // SETTERS
    virtual void setPostConnectAttributes();
    SQLRETURN setConnectAttr( SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nStringLength ); // make caller use our convenience methods by protecting this 

    // GETTERS
    SQLRETURN getConnectAttr( SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength, SQLINTEGER *pnRetSize ); // make caller use our convenience methods by protecting this 

    // DO'rs
};

#endif



