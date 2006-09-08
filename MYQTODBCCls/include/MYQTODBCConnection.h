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

#ifndef QTODBCCONNECTION_H
#define QTODBCCONNECTION_H

// libqt
#include <qobject.h>
#include <qptrlist.h>

// libODBC++
#include <MYODBCConnection.h>

//
#include <MYQTODBCStatement.h>

class MYQTODBCEnvironment;

//
#include "MYQTODBCLogin.h"
#include "MYQTODBCPromptDialog.h"
#include "MYQTODBCPrompt.h"

// extends MYODBCConnection
// - adds Qt signal/slot support

class MYQTODBCConnection : public QObject, public MYODBCConnection
{
    Q_OBJECT
public:
    MYQTODBCConnection( MYQTODBCEnvironment *penvironment );
    virtual ~MYQTODBCConnection();

    // SETTERS
    virtual void setPromptDriver( bool bPrompt ) { bPromptDriver = bPrompt; }
    virtual void setPromptDataSourceName( bool bPrompt ) { bPromptDataSourceName = bPrompt; }
    virtual void setPromptUserID( bool bPrompt ) { bPromptUserID = bPrompt; }
    virtual void setPromptPassword( bool bPrompt ) { bPromptPassword = bPrompt; }

    // GETTERS
    virtual MYQTODBCStatement *getExecute( const QString &stringSQL );
    virtual MYQTODBCStatement *getCatalogs();
    virtual MYQTODBCStatement *getSchemas( const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getTables( const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null, const QString &stringType = "TABLE" );
    virtual MYQTODBCStatement *getViews( const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null, const QString &stringType = "VIEW" );
    virtual MYQTODBCStatement *getColumns( const QString &stringTable = QString::null, const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null, const QString &stringType = "TABLE" );
    virtual MYQTODBCStatement *getIndexs( const QString &stringTable = QString::null, const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getPrimaryKeys( const QString &stringTable = QString::null, const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getForeignKeys( const QString &stringTable = QString::null, const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getSpecialColumns( const QString &stringTable = QString::null, const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getProcedures( const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getProcedureColumns( const QString &stringProcedure = QString::null, const QString &stringSchema = QString::null, const QString &stringCatalog = QString::null );
    virtual MYQTODBCStatement *getDataTypes();

    virtual const char *getZClassName() { return "MYQTODBCConnection"; }
    virtual bool getPromptDriver() { return bPromptDriver; }
    virtual bool getPromptDataSourceName() { return bPromptDataSourceName; }
    virtual bool getPromptUserID() { return bPromptUserID; }
    virtual bool getPromptPassword() { return bPromptPassword; }

    virtual QString getDSN() { return stringDSN; }
    virtual QString getUID() { return stringUID; }
    virtual QString getPWD() { return stringPWD; }

    virtual MYODBCMessage *getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState = 0, char *pszMessage = 0, SQLINTEGER nNativeCode = 0 );

    // DOERS
    // we replace MYODBCConnection versions of these so as to emit connect/disconnect signals
    virtual SQLRETURN doConnect( SQLCHAR *pszServerName = NULL, SQLSMALLINT nLength1 = SQL_NTS, SQLCHAR *pszUserName = NULL, SQLSMALLINT nLength2 = SQL_NTS, SQLCHAR *pszAuthentication = NULL, SQLSMALLINT nLength3 = SQL_NTS );
    virtual SQLRETURN doBrowseConnect( SQLCHAR *szInConnectionString, SQLSMALLINT nStringLength1, SQLCHAR *szOutConnectionString, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLength2Ptr );
    virtual SQLRETURN doDriverConnect( SQLHWND hWnd, SQLCHAR *pszIn, SQLSMALLINT nLengthIn, SQLCHAR *pszOut, SQLSMALLINT nLengthOut, SQLSMALLINT *pnLengthOut, SQLUSMALLINT nPrompt );
    virtual SQLRETURN doDisconnect();
    // we layer these on top of the above to support QString
    virtual SQLRETURN doConnect( const QString &stringServerName = QString::null, const QString &stringUserName = QString::null, const QString &stringAuthentication = QString::null );
    virtual SQLRETURN doBrowseConnect( const QString &stringIn, QString *pstringOut );
    virtual SQLRETURN doDriverConnect( SQLHWND hWnd, const QString &stringIn, QString *pstringOut, SQLUSMALLINT nPrompt );
    // we layer these on top of the above to provide QTODBC++ based prompting
    virtual bool doConnect( QWidget *pwidgetParent, const QString &stringServerName = QString::null, const QString &stringUserName = QString::null, const QString &stringAuthentication = QString::null );
    virtual bool doBrowseConnect( QWidget *pwidgetParent, const QString &stringConnect );
    virtual bool doBrowseConnect( QWidget *pwidgetParent );

signals:
    void signalMessage( MYODBCMessage *pmessage );
    void signalConnected();
    void signalDisconnected();

protected:
    // Used by doConnect() and doBrowseConnect()
    bool                bPromptDriver;
    bool                bPromptDataSourceName;
    // Used by doConnect()
    bool                bPromptUserID;
    bool                bPromptPassword;
    // These reflect the current connection. null if no connection.
    QString             stringDSN;
    QString             stringUID;
    QString             stringPWD;
    QString             stringConnectString;

    // Used by doBrowseConnect
    virtual QString getString( QPtrList<MYQTODBCPrompt> ptrlistPrompts );
    virtual QPtrList<MYQTODBCPrompt> getPromptList( const QString &string );
};

#endif



