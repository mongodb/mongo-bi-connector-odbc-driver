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

#include "MYQTODBCConnection.h"
#include "MYQTODBCStatement.h"

MYQTODBCStatement::MYQTODBCStatement( MYQTODBCConnection *pconnection )
    : QObject( 0, "MYQTODBCStatement" ), MYODBCStatement( pconnection )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    // echo up the object hierarchy
    connect( this, SIGNAL(signalMessage(MYODBCMessage*)), pconnection, SIGNAL(signalMessage(MYODBCMessage*)) );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCStatement::~MYQTODBCStatement()
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
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getExecute()
{
    SQLRETURN nReturn = MYODBCStatement::getExecute();
    if ( SQL_SUCCEEDED( nReturn ) && getHaveCursor() )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getExecDirect
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getExecDirect( SQLCHAR *pszStatement, SQLINTEGER nLength )
{
    SQLRETURN nReturn = MYODBCStatement::getExecDirect( pszStatement, nLength );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getCatalogs
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getCatalogs( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1 )
{
    SQLRETURN nReturn = MYODBCStatement::getTables( pszCatalogName, nLength1, (SQLCHAR*)"", SQL_NTS, (SQLCHAR*)"", SQL_NTS, (SQLCHAR*)"", SQL_NTS );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getSchemas
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getSchemas( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2 )
{
    SQLRETURN nReturn = MYODBCStatement::getTables( pszCatalogName, nLength1, pszSchemaName, nLength2, (SQLCHAR*)"", SQL_NTS, (SQLCHAR*)"", SQL_NTS );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getTables
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getTables( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLCHAR *pszTableType, SQLSMALLINT nLength4 )
{
    SQLRETURN nReturn = MYODBCStatement::getTables( pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, pszTableType, nLength4 );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getColumns
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getColumns( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLCHAR *pszColumnName, SQLSMALLINT nLength4 )
{
    SQLRETURN nReturn = MYODBCStatement::getColumns( pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, pszColumnName, nLength4 );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getStatistics
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getStatistics( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLUSMALLINT nUnique, SQLUSMALLINT nReserved )
{
    SQLRETURN nReturn = MYODBCStatement::getStatistics( pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, nUnique, nReserved );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getSpecialColumns
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getSpecialColumns( SQLSMALLINT nIdentifierType, SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3, SQLSMALLINT nScope, SQLSMALLINT nNullable )
{
    SQLRETURN nReturn = MYODBCStatement::getSpecialColumns( nIdentifierType, pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3, nScope, nNullable );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getPrimaryKeys
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getPrimaryKeys( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszTableName, SQLSMALLINT nLength3 )
{
    SQLRETURN nReturn = MYODBCStatement::getPrimaryKeys( pszCatalogName, nLength1, pszSchemaName, nLength2, pszTableName, nLength3 );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getForeignKeys
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getForeignKeys( SQLCHAR *pszPKCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszPKSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszPKTableName, SQLSMALLINT nLength3, SQLCHAR *pszFKCatalogName, SQLSMALLINT nLength4, SQLCHAR *pszFKSchemaName, SQLSMALLINT nLength5, SQLCHAR *pszFKTableName, SQLSMALLINT nLength6 )
{
    SQLRETURN nReturn = MYODBCStatement::getForeignKeys( pszPKCatalogName, nLength1, pszPKSchemaName, nLength2, pszPKTableName, nLength3, pszFKCatalogName, nLength4, pszFKSchemaName, nLength5, pszFKTableName, nLength6 );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getProcedures
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getProcedures( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszProcName, SQLSMALLINT nLength3 )
{
    SQLRETURN nReturn = MYODBCStatement::getProcedures( pszCatalogName, nLength1, pszSchemaName, nLength2, pszProcName, nLength3 );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getProcedureColumns
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getProcedureColumns( SQLCHAR *pszCatalogName, SQLSMALLINT nLength1, SQLCHAR *pszSchemaName, SQLSMALLINT nLength2, SQLCHAR *pszProcName, SQLSMALLINT nLength3, SQLCHAR *pszColumnName, SQLSMALLINT nLength4 )
{
    SQLRETURN nReturn = MYODBCStatement::getProcedureColumns( pszCatalogName, nLength1, pszSchemaName, nLength2, pszProcName, nLength3, pszColumnName, nLength4 );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getTypeInfo
    
    Replaces MYODBCStatement version so as to emit signals.
*/
SQLRETURN MYQTODBCStatement::getTypeInfo( SQLSMALLINT nDataType )
{
    SQLRETURN nReturn = MYODBCStatement::getTypeInfo( nDataType );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        emit signalElapsedSeconds( nElapsedSeconds );
        emit signalResults( this );
    }

    return nReturn;
}

/*!
    getExecDirect
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getExecDirect( const QString &stringStatement )
{
    SQLCHAR *pszStatement = (SQLCHAR*)stringStatement.latin1();
    int nStatement        = pszStatement ? SQL_NTS : 0;

    return getExecDirect( pszStatement, nStatement );
}

/*!
    getCatalogs
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getCatalogs( const QString &stringCatalog )
{
    SQLCHAR *   pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    int         nCatalog   = pszCatalog ? SQL_NTS : 0;

    return getCatalogs( pszCatalog, nCatalog );
}

/*!
    getSchemas
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getSchemas( const QString &stringCatalog, const QString &stringSchema )
{
    SQLCHAR *   pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *   pszSchema  = (SQLCHAR*)stringSchema.latin1();
    int         nCatalog   = pszCatalog ? SQL_NTS : 0;
    int         nSchema    = pszSchema ? SQL_NTS : 0; 

    return getSchemas( pszCatalog, nCatalog, pszSchema, nSchema );
}

/*!
    getTables
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getTables( const QString &stringCatalog, const QString &stringSchema, const QString &stringTable, const QString &stringType )
{
    SQLCHAR *pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema  = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszTable   = (SQLCHAR*)stringTable.latin1();
    SQLCHAR *pszType    = (SQLCHAR*)stringType.latin1();
    int nCatalog        = pszCatalog ? SQL_NTS : 0;
    int nSchema         = pszSchema ? SQL_NTS : 0; 
    int nTable          = pszTable ? SQL_NTS : 0;
    int nType           = pszType ? SQL_NTS : 0; 

    return getTables( pszCatalog, nCatalog, pszSchema, nSchema, pszTable, nTable, pszType, nType );
}

/*!
    getColumns
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getColumns( const QString &stringCatalog, const QString &stringSchema, const QString &stringTable, const QString &stringColumn )
{
    SQLCHAR *pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema  = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszTable   = (SQLCHAR*)stringTable.latin1();
    SQLCHAR *pszColumn  = (SQLCHAR*)stringColumn.latin1();
    int nCatalog        = pszCatalog ? SQL_NTS : 0;
    int nSchema         = pszSchema ? SQL_NTS : 0; 
    int nTable          = pszTable ? SQL_NTS : 0;
    int nColumn         = pszColumn ? SQL_NTS : 0; 

    return getColumns( pszCatalog, nCatalog, pszSchema, nSchema, pszTable, nTable, pszColumn, nColumn );
}

/*!
    getStatistics
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getStatistics( const QString &stringCatalog, const QString &stringSchema, const QString &stringTable, SQLUSMALLINT nUnique, SQLUSMALLINT nReserved )
{
    SQLCHAR *pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema  = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszTable   = (SQLCHAR*)stringTable.latin1();
    int nCatalog        = pszCatalog ? SQL_NTS : 0;
    int nSchema         = pszSchema ? SQL_NTS : 0; 
    int nTable          = pszTable ? SQL_NTS : 0;

    return getStatistics( pszCatalog, nCatalog, pszSchema, nSchema, pszTable, nTable, nUnique, nReserved );
}

/*!
    getSpecialColumns
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getSpecialColumns( SQLSMALLINT nIdentifierType, const QString &stringCatalog, const QString &stringSchema, const QString &stringTable, SQLSMALLINT nScope, SQLSMALLINT nNullable )
{
    SQLCHAR *pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema  = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszTable   = (SQLCHAR*)stringTable.latin1();
    int nCatalog        = pszCatalog ? SQL_NTS : 0;
    int nSchema         = pszSchema ? SQL_NTS : 0; 
    int nTable          = pszTable ? SQL_NTS : 0;

    return getSpecialColumns( nIdentifierType, pszCatalog, nCatalog, pszSchema, nSchema, pszTable, nTable, nScope, nNullable );
}

/*!
    getPrimaryKeys
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getPrimaryKeys( const QString &stringCatalog, const QString &stringSchema, const QString &stringTable )
{
    SQLCHAR *pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema  = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszTable   = (SQLCHAR*)stringTable.latin1();
    int nCatalog        = pszCatalog ? SQL_NTS : 0;
    int nSchema         = pszSchema ? SQL_NTS : 0; 
    int nTable          = pszTable ? SQL_NTS : 0;

    return getPrimaryKeys( pszCatalog, nCatalog, pszSchema, nSchema, pszTable, nTable );
}


/*!
    getForeignKeys
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getForeignKeys( const QString &stringPKCatalogName, const QString &stringPKSchemaName, const QString &stringPKTableName, const QString &stringFKCatalogName, const QString &stringFKSchemaName, const QString &stringFKTableName )
{
    SQLCHAR *pszPKCatalogName = (SQLCHAR*)stringPKCatalogName.latin1();
    SQLCHAR *pszPKSchemaName  = (SQLCHAR*)stringPKSchemaName.latin1();
    SQLCHAR *pszPKTableName   = (SQLCHAR*)stringPKTableName.latin1();

    SQLCHAR *pszFKCatalogName = (SQLCHAR*)stringFKCatalogName.latin1();
    SQLCHAR *pszFKSchemaName  = (SQLCHAR*)stringFKSchemaName.latin1();
    SQLCHAR *pszFKTableName   = (SQLCHAR*)stringFKTableName.latin1();

    int nPKCatalogName = pszPKCatalogName ? SQL_NTS : 0;
    int nPKSchemaName  = pszPKSchemaName ? SQL_NTS : 0;
    int nPKTableName   = pszPKTableName ? SQL_NTS : 0;

    int nFKCatalogName = pszFKCatalogName ? SQL_NTS : 0;
    int nFKSchemaName  = pszFKSchemaName ? SQL_NTS : 0;
    int nFKTableName   = pszFKTableName ? SQL_NTS : 0;

    return getForeignKeys( pszPKCatalogName, nPKCatalogName, pszPKSchemaName, nPKSchemaName, pszPKTableName, nPKTableName, pszFKCatalogName, nFKCatalogName, pszFKSchemaName, nFKSchemaName, pszFKTableName, nFKTableName );
}

/*!
    getProcedures
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getProcedures( const QString &stringCatalog, const QString &stringSchema, const QString &stringProc )
{
    SQLCHAR *pszCatalog = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema  = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszProc    = (SQLCHAR*)stringProc.latin1();
    int nCatalog        = pszCatalog ? SQL_NTS : 0;
    int nSchema         = pszSchema ? SQL_NTS : 0; 
    int nProc           = pszProc ? SQL_NTS : 0;

    return getProcedures( pszCatalog, nCatalog, pszSchema, nSchema, pszProc, nProc );
}

/*!
    getProcedureColumns
    
    Allows the use of QString instead of SQLCHAR* but otherwise simply
    calls the MYODBCStatement equivalent.
*/
SQLRETURN MYQTODBCStatement::getProcedureColumns( const QString &stringCatalog, const QString &stringSchema, const QString &stringProcedure, const QString &stringColumn )
{
    SQLCHAR *pszCatalog     = (SQLCHAR*)stringCatalog.latin1();
    SQLCHAR *pszSchema      = (SQLCHAR*)stringSchema.latin1();
    SQLCHAR *pszProcedure   = (SQLCHAR*)stringProcedure.latin1();
    SQLCHAR *pszColumn      = (SQLCHAR*)stringColumn.latin1();
    int nCatalog            = pszCatalog ? SQL_NTS : 0;
    int nSchema             = pszSchema ? SQL_NTS : 0; 
    int nProcedure          = pszProcedure ? SQL_NTS : 0;
    int nColumn             = pszColumn ? SQL_NTS : 0; 

    return getProcedureColumns( pszCatalog, nCatalog, pszSchema, nSchema, pszProcedure, nProcedure, pszColumn, nColumn );
}

/*!
    getNewMessage
    
    This replaces existing getNewMessage so we can emit signalMessage.
*/
MYODBCMessage *MYQTODBCStatement::getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState, char *pszMessage, SQLINTEGER nNativeCode )
{
    MYODBCMessage *pmessage = new MYODBCMessage( nType, pszState, pszMessage, nNativeCode );
    emit signalMessage( pmessage );
    return pmessage;
}

/*!
    getTables
    
    Query based upon a filter where filter may be of the form;
    
    [[CATALOG.]SCHEMA]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getTables( const QTODBCFilter &stringFilter, const QString &stringTableType )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringDummy;

    getParsedFilter( stringFilter, &stringDummy, &stringCatalog, &stringSchema );

    return getTables( stringCatalog, stringSchema, QString::null, stringTableType );
}

/*!
    getColumns
    
    Query based upon a filter where filter may be of the form;
    
    [[[CATALOG.]SCHEMA.]TABLE | VIEW]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getColumns( const QTODBCFilter &stringFilter )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringTable;

    getParsedFilter( stringFilter, &stringCatalog, &stringSchema, &stringTable );

    return getColumns( stringCatalog, stringSchema, stringTable );
}

/*!
    getStatistics
    
    Query based upon a filter where filter may be of the form;
    
    [[[CATALOG.]SCHEMA.]TABLE]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getStatistics( const QTODBCFilter &stringFilter, SQLUSMALLINT nUnique, SQLUSMALLINT nReserved )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringTable;

    getParsedFilter( stringFilter, &stringCatalog, &stringSchema, &stringTable );

    return getStatistics( stringCatalog, stringSchema, stringTable, nUnique, nReserved );
}

/*!
    getSpecialColumns
    
    Query based upon a filter where filter may be of the form;
    
    [[[CATALOG.]SCHEMA.]TABLE]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getSpecialColumns( const QTODBCFilter &stringFilter, SQLSMALLINT nIdentifierType, SQLSMALLINT nScope, SQLSMALLINT nNullable )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringTable;

    getParsedFilter( stringFilter, &stringCatalog, &stringSchema, &stringTable );

    return getSpecialColumns( nIdentifierType, stringCatalog, stringSchema, stringTable, nScope, nNullable );
}

/*!
    getPrimaryKeys
    
    Query based upon a filter where filter may be of the form;
    
    [[[CATALOG.]SCHEMA.]TABLE]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getPrimaryKeys( const QTODBCFilter &stringFilter )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringTable;

    getParsedFilter( stringFilter, &stringCatalog, &stringSchema, &stringTable );

    return getPrimaryKeys( stringCatalog, stringSchema, stringTable );
}

/*!
    getForeignKeys
    
    Query based upon a filter where filter may be of the form;
    
    [[[CATALOG.]SCHEMA.]TABLE]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getForeignKeys( const QTODBCFilter &stringFilter )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringTable;

    getParsedFilter( stringFilter, &stringCatalog, &stringSchema, &stringTable );

    return getForeignKeys( stringCatalog, stringSchema, stringTable );
}

/*!
    getProcedures
    
    Query based upon a filter where filter may be of the form;
    
    [[CATALOG.]SCHEMA]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getProcedures( const QTODBCFilter &stringFilter )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringDummy;

    getParsedFilter( stringFilter, &stringDummy, &stringCatalog, &stringSchema );

    return getProcedures( stringCatalog, stringSchema );
}

/*!
    getProcedureColumns
    
    Query based upon a filter where filter may be of the form;
    
    [[[CATALOG.]SCHEMA.]PROCEDURE]
    
    The filter is expanded and the query is executed.
*/
SQLRETURN MYQTODBCStatement::getProcedureColumns( const QTODBCFilter &stringFilter )
{
    QString stringCatalog;
    QString stringSchema;
    QString stringProcedure;

    getParsedFilter( stringFilter, &stringCatalog, &stringSchema, &stringProcedure );

    return getProcedureColumns( stringCatalog, stringSchema, stringProcedure );
}

/*!
    getParsedFilter

    Will seperate a stringFilter which is expected to be of the form;
    
    [[[CATALOG.]SCHEMA.]OBJECT]
*/
void MYQTODBCStatement::getParsedFilter(  const QTODBCFilter &stringFilter, QString *pstringCatalog, QString *pstringSchema, QString *pstringObject )
{
    *pstringCatalog = QString::null;
    *pstringSchema  = QString::null;
    *pstringObject  = QString::null;

    QStringList stringlist = QStringList::split( '.', stringFilter );
    if ( stringlist.count() > 0 )
        *pstringObject = stringlist[stringlist.count() - 1];

    if ( stringlist.count() > 1 )
        *pstringSchema = stringlist[stringlist.count() - 2];

    if ( stringlist.count() > 2 )
        *pstringCatalog = stringlist[stringlist.count() - 3];
}

/*!
    doWaiting
    
    Replaces MYODBCStatement::doWaiting. 
    
    This will give time to the main Qt event loop during any asynch operation such
    that the application gets some time to do things like; repaint the screen or update
    a progress dialog.
*/
bool MYQTODBCStatement::doWaiting()
{
    // I have this printf in here because I have not, yet, found a driver which supports asynch option.
    printf( "[PAH][%s][%d] Looks like asynch option supported by this driver.\n", __FILE__, __LINE__ );
    qApp->processEvents();

    return true;
}

/*!
    slotExecute
    
    Executes the given statement.
*/
SQLRETURN MYQTODBCStatement::slotExecute( const QString &stringSQL )
{
    if ( stringSQL.isEmpty() )
    {
        objectlistMessages.doAppend( new MYODBCMessage( MYODBCMessage::MessageError, 0, "[QTODBC++] slotExecute(): SQL statement is empty", -1 ) );
        return SQL_ERROR;
    }

    SQLRETURN nReturn;
     
    // Use doPrepare or call doExecDirect ???? There would seem to be diff parse logic in some drivers (i.e. DB2) re. multi-statements and comments

    nReturn = doPrepare( (SQLCHAR*)(stringSQL.latin1()), SQL_NTS );

    if ( !SQL_SUCCEEDED( nReturn ) )
        return nReturn;

    nReturn = getExecute();

    return nReturn;
}


