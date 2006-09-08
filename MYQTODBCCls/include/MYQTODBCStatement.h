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

#ifndef QTODBCSTATEMENT_H
#define QTODBCSTATEMENT_H

// libqt
#include <qobject.h>
#include <qapplication.h>

// libODBC++
#include <MYODBCStatement.h>

class MYQTODBCConnection;

class QTODBCFilter : public QString
{
public:
    QTODBCFilter( const QString &string )
        : QString( string )
    {
    }

};

// extends MYODBCStatement
// - adds Qt signal/slot support
// - adds Qt thread support
// - allows the use of some Qt data types as args

class MYQTODBCStatement : public QObject, public MYODBCStatement
{
    Q_OBJECT
public:
    MYQTODBCStatement( MYQTODBCConnection *pconnection );
    virtual ~MYQTODBCStatement();

    // SETTERS

    // GETTERS
    // to ensure Qt signals emitted
    virtual SQLRETURN getExecute();
    virtual SQLRETURN getExecDirect( SQLCHAR *pszStatement, SQLINTEGER nLength = SQL_NTS );
    virtual SQLRETURN getCatalogs( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0 );
    virtual SQLRETURN getSchemas( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0 );
    virtual SQLRETURN getTables( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszTableType = NULL, SQLSMALLINT nLength4 = 0 );
    virtual SQLRETURN getColumns( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszColumnName = NULL, SQLSMALLINT nLength4 = 0 );
    virtual SQLRETURN getStatistics( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLUSMALLINT nUnique = SQL_INDEX_ALL, SQLUSMALLINT nReserved = SQL_QUICK );
    virtual SQLRETURN getSpecialColumns( SQLSMALLINT nIdentifierType = SQL_BEST_ROWID, SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0, SQLSMALLINT nScope = SQL_SCOPE_CURROW, SQLSMALLINT nNullable = SQL_NULLABLE );
    virtual SQLRETURN getPrimaryKeys( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszTableName = NULL, SQLSMALLINT nLength3 = 0 );
    virtual SQLRETURN getForeignKeys( SQLCHAR *pszPKCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszPKSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszPKTableName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszFKCatalogName = NULL, SQLSMALLINT nLength4 = 0, SQLCHAR *pszFKSchemaName = NULL, SQLSMALLINT nLength5 = 0, SQLCHAR *pszFKTableName = NULL, SQLSMALLINT nLength6 = 0 );
    virtual SQLRETURN getProcedures( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszProcName = NULL, SQLSMALLINT nLength3 = 0 );
    virtual SQLRETURN getProcedureColumns( SQLCHAR *pszCatalogName = NULL, SQLSMALLINT nLength1 = 0, SQLCHAR *pszSchemaName = NULL, SQLSMALLINT nLength2 = 0, SQLCHAR *pszProcName = NULL, SQLSMALLINT nLength3 = 0, SQLCHAR *pszColumnName = NULL, SQLSMALLINT nLength4 = 0 );
    virtual SQLRETURN getTypeInfo( SQLSMALLINT nDataType = SQL_ALL_TYPES );

    // to support QString
    virtual SQLRETURN getExecDirect( const QString &stringStatement );
    virtual SQLRETURN getCatalogs( const QString &stringCatalogName );
    virtual SQLRETURN getSchemas( const QString &stringCatalogName, const QString &stringSchemaName = QString::null );
    virtual SQLRETURN getTables( const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringTableName = QString::null, const QString &stringTableType = QString::null );
    virtual SQLRETURN getColumns(  const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringTableName = QString::null, const QString &stringColumnName = QString::null );
    virtual SQLRETURN getStatistics( const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringTableName = QString::null, SQLUSMALLINT nUnique = SQL_INDEX_ALL, SQLUSMALLINT nReserved = SQL_QUICK );
    virtual SQLRETURN getSpecialColumns( SQLSMALLINT nIdentifierType, const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringTableName = QString::null, SQLSMALLINT nScope = SQL_SCOPE_CURROW, SQLSMALLINT nNullable = SQL_NULLABLE );
    virtual SQLRETURN getPrimaryKeys( const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringTableName = QString::null );
    virtual SQLRETURN getForeignKeys( const QString &stringPKCatalogName, const QString &stringPKSchemaName = QString::null, const QString &stringPKTableName = QString::null, const QString &stringFKCatalogName = QString::null, const QString &stringFKSchemaName = QString::null, const QString &stringFKTableName = QString::null );
    virtual SQLRETURN getProcedures( const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringProcName = QString::null );
    virtual SQLRETURN getProcedureColumns( const QString &stringCatalogName, const QString &stringSchemaName = QString::null, const QString &stringProcName = QString::null, const QString &stringColumnName = QString::null );
    virtual MYODBCMessage *getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState = 0, char *pszMessage = 0, SQLINTEGER nNativeCode = 0 );

    // to support a filter string
    virtual SQLRETURN getTables( const QTODBCFilter &stringFilter, const QString &stringTableType = QString::null );
    virtual SQLRETURN getColumns( const QTODBCFilter &stringFilter );
    virtual SQLRETURN getStatistics( const QTODBCFilter &stringFilter, SQLUSMALLINT nUnique = SQL_INDEX_ALL, SQLUSMALLINT nReserved = SQL_QUICK );
    virtual SQLRETURN getSpecialColumns( const QTODBCFilter &stringFilter, SQLSMALLINT nIdentifierType = SQL_BEST_ROWID, SQLSMALLINT nScope = SQL_SCOPE_CURROW, SQLSMALLINT nNullable = SQL_NULLABLE );
    virtual SQLRETURN getPrimaryKeys( const QTODBCFilter &stringFilter );
    virtual SQLRETURN getForeignKeys( const QTODBCFilter &stringFilter );
    virtual SQLRETURN getProcedures( const QTODBCFilter &stringFilter );
    virtual SQLRETURN getProcedureColumns( const QTODBCFilter &stringFilter );

    //
    static void getParsedFilter( const QTODBCFilter &stringFilter, QString *pstringCatalog, QString *pstringSchema, QString *pstringObject );

    // DOERS
    virtual bool doWaiting();

signals:
    void signalMessage( MYODBCMessage *pmessage );
    void signalElapsedSeconds( double nElapsedSeconds );
    void signalResults( MYQTODBCStatement *pstatement );

public slots:
    SQLRETURN slotExecute( const QString &stringSQL );

};

#endif



