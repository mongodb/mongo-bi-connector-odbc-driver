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

#ifndef ODBCENVIRONMENT_H
#define ODBCENVIRONMENT_H

//
#include "MYODBCObject.h"
#include "MYODBCConnection.h"
#include "MYODBCMessage.h"

// wraps SQLHENV

class MYODBCEnvironment : public MYODBCObject
{
public:
    MYODBCEnvironment();
    virtual ~MYODBCEnvironment();

    // SETTERS

    // GETTERS
    virtual SQLRETURN getDriver( SQLUSMALLINT nDirection, SQLCHAR *nDriverDescription, SQLSMALLINT nBufferLength1, SQLSMALLINT *pnDescriptionLengthPtr, SQLCHAR *pszDriverAttributes, SQLSMALLINT nBufferLength2, SQLSMALLINT *pnAttributesLengthPtr );
    virtual SQLRETURN getDataSource( SQLUSMALLINT nDirection, SQLCHAR *pszServerName, SQLSMALLINT nBufferLength1, SQLSMALLINT *pnNameLength1Ptr, SQLCHAR *pszDescription, SQLSMALLINT nBufferLength2, SQLSMALLINT *pnNameLength2Ptr );

    virtual SQLHENV getEnv() { return hEnv; }
    virtual const char *getZClassName() { return "MYODBCEnvironment"; }
    virtual MYODBCMessage *getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState = 0, char *pszMessage = 0, SQLINTEGER nNativeCode = 0 );
    virtual MYODBCObjectList *getMessages() { return &objectlistMessages; }

    // DO'rs
    virtual BOOL doManageDataSources( HWND hWnd );
    virtual BOOL doAlloc();        
    virtual BOOL doFree();         
    virtual void doErrors( MYODBCMessage::MessageTypes nType );

protected:
    SQLHENV	        hEnv;
    MYODBCObjectList  objectlistMessages;
};

#endif



