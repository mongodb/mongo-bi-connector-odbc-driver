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

#include <MYQTODBCEnvironment.h>
#include <MYQTODBCConnection.h>

MYQTODBCConnection::MYQTODBCConnection( MYQTODBCEnvironment *penvironment )
    : QObject( 0, "MYQTODBCConnection" ), MYODBCConnection( penvironment )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    bPromptDriver           = true;
    bPromptDataSourceName   = true;
    bPromptUserID           = true;
    bPromptPassword         = true;

    // echo up the object hierarchy
    connect( this, SIGNAL(signalMessage(MYODBCMessage*)), penvironment, SIGNAL(signalMessage(MYODBCMessage*)) );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCConnection::~MYQTODBCConnection()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    getExecute
    
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getTables.
*/
MYQTODBCStatement *MYQTODBCConnection::getExecute( const QString &stringSQL )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->slotExecute( stringSQL );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getCatalogs
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getTables.
*/
MYQTODBCStatement *MYQTODBCConnection::getCatalogs()
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getTables( SQL_ALL_CATALOGS, QString::null, QString::null );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getSchemas
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getTables.
*/
MYQTODBCStatement *MYQTODBCConnection::getSchemas( const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getTables( stringCatalog, SQL_ALL_SCHEMAS, QString::null );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getTables
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getTables.
*/
MYQTODBCStatement *MYQTODBCConnection::getTables( const QString &stringSchema, const QString &stringCatalog, const QString &stringType )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getTables( stringCatalog, stringSchema, SQL_ALL_SCHEMAS, stringType );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getViews
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getTables.
*/
MYQTODBCStatement *MYQTODBCConnection::getViews( const QString &stringSchema, const QString &stringCatalog, const QString &stringType )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getTables( stringCatalog, stringSchema, SQL_ALL_SCHEMAS, stringType );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getColumns
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getColumns.
*/
MYQTODBCStatement *MYQTODBCConnection::getColumns( const QString &stringTable, const QString &stringSchema, const QString &stringCatalog, const QString &stringType )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getColumns( stringCatalog, stringSchema, stringTable, stringType );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getIndexs
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getStatistics.
*/
MYQTODBCStatement *MYQTODBCConnection::getIndexs( const QString &stringTable, const QString &stringSchema, const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getStatistics( stringCatalog, stringSchema, stringTable );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getPrimaryKey
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getPrimaryKey.
*/
MYQTODBCStatement *MYQTODBCConnection::getPrimaryKeys( const QString &stringTable, const QString &stringSchema, const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getPrimaryKeys( stringCatalog, stringSchema, stringTable );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getForeignKeys
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getForeignKeys.
*/
MYQTODBCStatement *MYQTODBCConnection::getForeignKeys( const QString &stringTable, const QString &stringSchema, const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getForeignKeys( stringCatalog, stringSchema, stringTable );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getSpecialColumns
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getSpecialColumns.
*/
MYQTODBCStatement *MYQTODBCConnection::getSpecialColumns( const QString &stringTable, const QString &stringSchema, const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getSpecialColumns( SQL_BEST_ROWID, stringCatalog, stringSchema, stringTable );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getProcedures
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getProcedures.
*/
MYQTODBCStatement *MYQTODBCConnection::getProcedures( const QString &stringSchema, const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getProcedures( stringCatalog, stringSchema );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getProcedureColumns
        
    Creates a result set (or NULL if there was an error).
    See MYQTODBCStatement::getProcedureColumns.
*/
MYQTODBCStatement *MYQTODBCConnection::getProcedureColumns( const QString &stringProcedure, const QString &stringSchema, const QString &stringCatalog )
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getProcedureColumns( stringCatalog, stringSchema, stringProcedure );
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}

/*!
    getDataTypes
        
    Creates a result set (or NULL if there was an error).
    See ODBCStatement::getypeInfo.
*/
MYQTODBCStatement *MYQTODBCConnection::getDataTypes()
{
    MYQTODBCStatement *   pstatement = 0;
    SQLRETURN           nReturn;

    if ( hDbc == SQL_NULL_HDBC )
    {
        if ( !SQL_SUCCEEDED( ( nReturn = doAlloc() ) ) )
            return 0;
    }

    pstatement = new MYQTODBCStatement( this );
    nReturn = pstatement->getTypeInfo();
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        delete pstatement;
        pstatement = 0;
    }

    return pstatement;
}



/*!
    getNewMessage
    
    This replaces existing getNewMessage so we can emit signalMessage.
*/
MYODBCMessage *MYQTODBCConnection::getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState, char *pszMessage, SQLINTEGER nNativeCode )
{
    MYODBCMessage *pmessage = new MYODBCMessage( nType, pszState, pszMessage, nNativeCode );
    emit signalMessage( pmessage );
    return pmessage;
}

/*!
    doConnect
    
    This replaces ODBCConnection::doConnect but does the same thing. This
    method also emits signalConnected() if things worked out.
*/
SQLRETURN MYQTODBCConnection::doConnect( SQLCHAR *pszServerName, SQLSMALLINT nLength1, SQLCHAR *pszUserName, SQLSMALLINT nLength2, SQLCHAR *pszAuthentication, SQLSMALLINT nLength3 )
{
    SQLRETURN nReturn = MYODBCConnection::doConnect( pszServerName, nLength1, pszUserName, nLength2, pszAuthentication, nLength3 );

    if ( getIsConnected() )
    {
        stringDSN = (const char *)pszServerName;
        stringUID = (const char *)pszUserName;
        stringPWD = (const char *)pszAuthentication;
        emit signalConnected();
    }

    return nReturn;
}

/*!
    doBrowseConnect
    
    This replaces MYODBCConnection::doBrowseConnect but does the same thing. This
    method also emits signalConnected() if things worked out.
*/
SQLRETURN MYQTODBCConnection::doBrowseConnect( SQLCHAR *szInConnectionString, SQLSMALLINT nStringLength1, SQLCHAR *szOutConnectionString, SQLSMALLINT nBufferLength, SQLSMALLINT *pnStringLength2Ptr )
{
    SQLRETURN nReturn = MYODBCConnection::doBrowseConnect( szInConnectionString, nStringLength1, szOutConnectionString, nBufferLength, pnStringLength2Ptr );

    if ( getIsConnected() )
    {
        stringConnectString = (const char *)szOutConnectionString;
        emit signalConnected();
    }

    return nReturn;
}

/*!
    doDriverConnect
    
    This replaces MYODBCConnection::doDriverConnect but does the same thing. This
    method also emits signalConnected() if things worked out.
*/
SQLRETURN MYQTODBCConnection::doDriverConnect( SQLHWND hWnd, SQLCHAR *pszIn, SQLSMALLINT nLengthIn, SQLCHAR *pszOut, SQLSMALLINT nLengthOut, SQLSMALLINT *pnLengthOut, SQLUSMALLINT nPrompt )
{
    SQLRETURN nReturn = MYODBCConnection::doDriverConnect( hWnd, pszIn, nLengthIn, pszOut, nLengthOut, pnLengthOut, nPrompt );

    if ( getIsConnected() )
        emit signalConnected();

    return nReturn;
}

/*!
    doDisconnect
    
    This replaces ODBCCOnnection::doDisconnect but does the same thing. This
    method also emits signalDisconnected() if things worked out.
*/
SQLRETURN MYQTODBCConnection::doDisconnect()
{
    SQLRETURN nReturn = MYODBCConnection::doDisconnect();

    if ( !getIsConnected() )
    {
        stringDSN           = QString::null;
        stringUID           = QString::null;
        stringPWD           = QString::null;
        stringConnectString = QString::null;
        emit signalDisconnected();
    }

    return nReturn;
}

/*!
    doConnect

    Allows the use of QString instead of SQLCHAR* but otherwise does
    same thing.
*/
SQLRETURN MYQTODBCConnection::doConnect( const QString &stringDSN, const QString &stringUID, const QString &stringPWD )
{
    SQLCHAR *pszDSN = (SQLCHAR*)"";
    SQLCHAR *pszUID = (SQLCHAR*)"";
    SQLCHAR *pszPWD = (SQLCHAR*)"";

    if ( !stringDSN.isEmpty() )
        pszDSN = (SQLCHAR*)stringDSN.latin1();
    if ( !stringUID.isEmpty() )
        pszUID = (SQLCHAR*)stringUID.latin1();
    if ( !stringPWD.isEmpty() )
        pszPWD = (SQLCHAR*)stringPWD.latin1();

    return doConnect( pszDSN, SQL_NTS, pszUID, SQL_NTS, pszPWD, SQL_NTS );
}

/*!
    doBrowseConnect
    
    Allows the use of QString instead of SQLCHAR* but otherwise does
    same thing.
*/
SQLRETURN MYQTODBCConnection::doBrowseConnect( const QString &stringIn, QString *pstringOut )
{
    SQLCHAR *   pszIn       = (SQLCHAR*)"";
    SQLCHAR     szOut[4096];
    SQLSMALLINT nLengthIn   = 0;
    SQLSMALLINT nLengthOut;

    *szOut = '\0';

    if ( !stringIn.isEmpty() )
    {
        pszIn       = (SQLCHAR*)stringIn.latin1();
        nLengthIn   = stringIn.length();
    }

    SQLRETURN nReturn = doBrowseConnect( pszIn, nLengthIn, szOut, sizeof(szOut), &nLengthOut ); 
    if ( nReturn == SQL_NEED_DATA )
    {
        *pstringOut = (const char*)szOut;
    }

    return nReturn;
}

/*!
    doDriverConnect

    Allows the use of QString instead of SQLCHAR* but otherwise does
    same thing.
    
    NOTE: We do not have QTODBC++ based prompting for Driver Connect because 
          the whole reason for using it is to have the Driver do the 
          prompting. The exception to this is that the Driver Manager will
          prompt for DSN, DRV, or FILEDSN if the stringIn is empty. In anycase;
          QTODBC++ probably should NOT do any prompting.
          
          The down side of this is that applications calling a Driver Connect
          method are unlikley to be portable without the application taking
          further measures. This is because the Driver Manager and the Driver
          may not support any kind of prompting - often the case on UNIX.
*/
SQLRETURN MYQTODBCConnection::doDriverConnect( SQLHWND hWnd, const QString &stringIn, QString *pstringOut, SQLUSMALLINT nPrompt )
{
    SQLCHAR *   pszIn       = (SQLCHAR*)"";
    SQLCHAR     szOut[4096];
    SQLSMALLINT nLengthIn   = 0;
    SQLSMALLINT nLengthOut;

    if ( !stringIn.isEmpty() )
    {
        pszIn       = (SQLCHAR*)stringIn.latin1();
        nLengthIn   = stringIn.length();
    }

    SQLRETURN nReturn = doDriverConnect( hWnd, pszIn, nLengthIn, szOut, sizeof(szOut), &nLengthOut, nPrompt ); 
    if ( nReturn == SQL_NEED_DATA )
    {
        *pstringOut = (const char*)szOut;
    }

    return nReturn;
}

/*!
    doConnect
    
    Allows the use of QString instead of SQLCHAR*.
    
    QTODBC++ will provide a login dialog.
*/
bool MYQTODBCConnection::doConnect( QWidget *pwidgetParent, const QString &stringDSN, const QString &stringUID, const QString &stringPWD )
{
    bool bReturn = false;

    // With Prompting.
    MYQTODBCLogin *plogin = new MYQTODBCLogin( pwidgetParent, (MYQTODBCEnvironment*)penvironment );
    plogin->setCaption( "Connect..." );
    plogin->setShowDriver( false );
    bool bPromptSomething = bPromptDataSourceName || bPromptUserID || bPromptPassword;
    if ( !bPromptDataSourceName ) plogin->setShowDataSourceName( false );
    if ( !bPromptUserID ) plogin->setShowUserID( false );
    if ( !bPromptPassword ) plogin->setShowPassword( false );
    if ( !stringDSN.isNull() )
        plogin->setDataSourceName( stringDSN );
    if ( !stringUID.isNull() )
        plogin->setUserID( stringUID );
    if ( ! stringPWD.isNull() )
        plogin->setPassword( stringPWD );
    while ( 1 )
    {
        if ( !bPromptSomething || (plogin->exec() == QDialog::Accepted) ) 
        {
            SQLRETURN nReturn = doConnect( plogin->getDataSourceName(), plogin->getUserID(), plogin->getPassword() );
            if ( SQL_SUCCEEDED( nReturn ) )
            {
                bReturn = true;
                break;
            }
            else
            {
                // if login failed - prompt for all
                plogin->setShowDataSourceName( true );
                plogin->setShowUserID( true );
                plogin->setShowPassword( true );
                bPromptSomething = true;
                QMessageBox( pwidgetParent, "failed to connect" ); 
            }
        }
        else
            break;
    }
    delete plogin;

    return bReturn;
}

/*!
    doBrowseConnect
    
    This method assumes that we have been given one of the following in stringConnect;
    
        "DRIVER=drivername"
        "DSN=dsnname"
        "FILEDSN=filename"

    Other connect options may be included.
    
    Call doBrowseConnect( QWidget * ) if you want QTODBC++ to prompt for the above 
    connect options as well.
            
    stringConnect is used to start the browse connect process.
    
    This method will use QTODBC++ based prompting as required.
*/
bool MYQTODBCConnection::doBrowseConnect( QWidget *pwidgetParent, const QString &stringConnect )
{
    bool        bReturn     = false;
    SQLRETURN   nReturn     = SQL_NEED_DATA;
    QString     stringIn    = stringConnect;
    QString     stringOut   = "";

    while ( nReturn == SQL_NEED_DATA )
    {
        nReturn = doBrowseConnect( stringIn, &stringOut );
        switch ( nReturn )
        {
            case SQL_SUCCESS:
            case SQL_SUCCESS_WITH_INFO:
                bReturn = true;
                break;
            case SQL_NEED_DATA:
                {
                    // prepare promptlist
                    QPtrList<MYQTODBCPrompt> ptrlistPrompts = getPromptList( stringOut );
                    ptrlistPrompts.setAutoDelete( true );

                    // prompt
                    MYQTODBCPromptDialog *ppromptdialog = new MYQTODBCPromptDialog( pwidgetParent, ptrlistPrompts );
                    // prepare new stringIn
                    if ( ppromptdialog->exec() == QDialog::Accepted )
                    {
                        stringIn = getString( ptrlistPrompts );
                    }
                    // handle cancel
                    else
                        nReturn = SQL_ERROR;

                    // cleanup prompt stuff
                    ptrlistPrompts.clear();
                    delete ppromptdialog;
                }
                break;
            case SQL_ERROR:
            case SQL_INVALID_HANDLE:
            default:
                break;
        }
    }

    return bReturn;
}

/*!
    doBrowseConnect
    
    This method will prompt for Driver/DSN/FileDSN before starting the regular 
    browse connect process.

    This method will use QTODBC++ based prompting.
*/
bool MYQTODBCConnection::doBrowseConnect( QWidget *pwidgetParent )
{
    bool bReturn = false;
    
    MYQTODBCLogin *plogin = new MYQTODBCLogin( pwidgetParent, (MYQTODBCEnvironment*)penvironment );
    plogin->setCaption( "Browse Connect..." );
    while ( 1 )
    {
        if ( plogin->exec() == QDialog::Accepted ) 
        {
            QString stringConnection;
            QString stringDriver            = plogin->getDriver();
            QString stringDataSourceName    = plogin->getDataSourceName();
            QString stringUserID            = plogin->getUserID();
            QString stringPassword          = plogin->getPassword();

            if ( stringDriver.isEmpty() && stringDataSourceName.isEmpty() )
                QMessageBox( pwidgetParent, "please select a Driver or Data Source Name" ); 
            else
            {
                delete plogin;

                if ( !stringDriver.isEmpty() )
                    stringConnection += "DRIVER=" + stringDriver + ";";
//                if ( !stringFileDataSourceName.isEmpty() )
//                    stringConnection += "FILEDSN=" + stringFileDataSourceName + ";";
                if ( !stringDataSourceName.isEmpty() )
                    stringConnection += "DSN=" + stringDataSourceName + ";";
                if ( !stringUserID.isEmpty() )
                    stringConnection += "UID=" + stringUserID + ";";
                if ( !stringPassword.isEmpty() )
                    stringConnection += "PWD=" + stringPassword + ";";

                return doBrowseConnect( pwidgetParent, stringConnection );
            }
        }
        else
        {
            delete plogin;
            break;
        }
    }

    return bReturn;
}

/*!
    getString
    
    Return a string list suitable for an *input* connection string for a browse connect.
    
    A browse request connection string has the following syntax:
    
    connection-string ::= attribute[;] | attribute; connection-string
    attribute ::= attribute-keyword=attribute-value | DRIVER=[{]attribute-value[}]
    attribute-keyword ::= DSN | UID | PWD
              | driver-defined-attribute-keyword
    attribute-value ::= character-string
    driver-defined-attribute-keyword ::= identifier
    
    where character-string has zero or more characters; identifier has one or more characters; 
    attribute-keyword is not case-sensitive; attribute-value may be case-sensitive; and the 
    value of the DSN keyword does not consist solely of blanks. Because of connection string 
    and initialization file grammar, keywords and attribute values that contain the characters 
    []{}(),;?*=!@ should be avoided. Because of the grammar in the system information, 
    keywords and data source names cannot contain the backslash (\) character. For an ODBC 
    2.x driver, braces are required around the attribute value for the DRIVER keyword.
    
    If any keywords are repeated in the browse request connection string, the driver uses the 
    value associated with the first occurrence of the keyword. If the DSN and DRIVER keywords 
    are included in the same browse request connection string, the Driver Manager and driver 
    use whichever keyword appears first.
    
    For information about how an application chooses a data source or driver, see 
    "Choosing a Data Source or Driver" in Chapter 6: Connecting to a Data Source or Driver.
*/
QString MYQTODBCConnection::getString( QPtrList<MYQTODBCPrompt> ptrlistPrompts )
{
    QString                 stringReturn;
    MYQTODBCPrompt *          pprompt           = 0;
    int                     nPrompt           = 0;
    int                     nPrompts          = ptrlistPrompts.count();

    for ( ; nPrompt < nPrompts; nPrompt++ )
    {
        pprompt         = ptrlistPrompts.at( nPrompt );
        stringReturn    += pprompt->getName() + "=";
        stringReturn    += pprompt->getValue() + ";";
    }

    return stringReturn;
}

/*!
    getPromptList
    
    Return a list of prompts based upon a string list returned from a browse connect call.
    
    The browse result connection string is a list of connection attributes. A connection attribute 
    consists of an attribute keyword and a corresponding attribute value. The browse result 
    connection string has the following syntax:
    
    connection-string ::= attribute[;] | attribute; connection-string
    attribute ::= [*]attribute-keyword=attribute-value
    attribute-keyword ::= ODBC-attribute-keyword
              | driver-defined-attribute-keyword
    ODBC-attribute-keyword = {UID | PWD}[:localized-identifier]
    driver-defined-attribute-keyword ::= identifier[:localized-identifier]
    attribute-value ::= {attribute-value-list} | ?
    (The braces are literal; they are returned by the driver.)
    attribute-value-list ::= character-string [:localized-character string] | character-string 
    [:localized-character string], attribute-value-list
    
    where character-string and localized-character string have zero or more characters; 
    identifier and localized-identifier have one or more characters; attribute-keyword is not 
    case-sensitive; and attribute-value may be case-sensitive. Because of connection string and 
    initialization file grammar, keywords, localized identifiers, and attribute values that 
    contain the characters []{}(),;?*=!@ should be avoided. Because of the grammar in the system 
    information, keywords and data source names cannot contain the backslash (\) character.
    
    The browse result connection string syntax is used according to the following semantic rules:
    
        * If an asterisk (*) precedes an attribute-keyword, the attribute is optional and can be 
          omitted in the next call to SQLBrowseConnect.
        * The attribute keywords UID and PWD have the same meaning as defined in SQLDriverConnect.
        * A driver-defined-attribute-keyword names the kind of attribute for which an attribute 
          value may be supplied. For example, it might be SERVER, DATABASE, HOST, or DBMS.
        * ODBC-attribute-keywords and driver-defined-attribute-keywords include a localized or 
          user-friendly version of the keyword. This might be used by applications as a label in a 
          dialog box. However, UID, PWD, or the identifier alone must be used when passing a browse 
          request string to the driver.
        * The {attribute-value-list} is an enumeration of actual values valid for the corresponding 
          attribute-keyword. Note that the braces ({}) do not indicate a list of choices; they are 
          returned by the driver. For example, it might be a list of server names or a list of database 
          names.
        * If the attribute-value is a single question mark (?), a single value corresponds to the 
          attribute-keyword. For example, UID=JohnS; PWD=Sesame.
        * Each call to SQLBrowseConnect returns only the information required to satisfy the next 
          level of the connection process. The driver associates state information with the 
          connection handle so that the context can always be determined on each call.

*/
QPtrList<MYQTODBCPrompt> MYQTODBCConnection::getPromptList( const QString &string )
{
    QPtrList<MYQTODBCPrompt> ptrlistPrompts;
    
    QStringList stringlist = QStringList::split( ';', string, false );

    for ( QStringList::Iterator it = stringlist.begin(); it != stringlist.end(); ++it ) 
    {
        QStringList stringlistName = QStringList::split( ':', *it, true );
        if ( stringlistName.count() > 0 )
        {
            MYQTODBCPrompt *pprompt = new MYQTODBCPrompt( MYQTODBCPrompt::PromptLineEdit, stringlistName[0] );
            if ( stringlistName.count() > 1 )
                pprompt->setValue( stringlistName[1] );
            ptrlistPrompts.append( pprompt );
        }
    }

    return ptrlistPrompts;
}

