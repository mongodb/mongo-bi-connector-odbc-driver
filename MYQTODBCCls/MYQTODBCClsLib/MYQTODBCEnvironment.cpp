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

#include "MYQTODBCEnvironment.h"

MYQTODBCEnvironment::MYQTODBCEnvironment()
    : QObject( 0, "MYQTODBCEnvironment" ), MYODBCEnvironment()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif


#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYQTODBCEnvironment::~MYQTODBCEnvironment()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    Qt friendly way to get a list of DSN's. 
*/
SQLRETURN MYQTODBCEnvironment::getDataSources( QStringList *pstringlist, bool bUser, bool bSystem )
{
    SQLRETURN       nReturn;
    SQLUSMALLINT    nDirection  =   SQL_FETCH_FIRST;
    SQLCHAR         szDSN[101];
    SQLSMALLINT     nLength1;
    SQLCHAR         szDescription[101];
    SQLSMALLINT     nLength2;

    if ( !bUser && !bSystem )
        return SQL_NO_DATA;

#ifndef Q_WS_MACX
    if ( !bUser && bSystem )
        nDirection = SQL_FETCH_FIRST_SYSTEM;
    else if ( bUser && !bSystem )
        nDirection = SQL_FETCH_FIRST_USER;
#endif

    nReturn = getDataSource( nDirection, szDSN, sizeof(szDSN) - 1, &nLength1, szDescription, sizeof(szDescription) - 1, &nLength2 );
    while ( SQL_SUCCEEDED( nReturn ) )
    {
        (*pstringlist) += (const char*)szDSN;
        nReturn = getDataSource( SQL_FETCH_NEXT, szDSN, sizeof(szDSN) - 1, &nLength1, szDescription, sizeof(szDescription) - 1, &nLength2 );
    }

    return nReturn;
}

/*!
    getNewMessage
    
    This replaces existing getNewMessage so we can emit signalMessage.
*/
MYODBCMessage *MYQTODBCEnvironment::getNewMessage( MYODBCMessage::MessageTypes nType, char *pszState, char *pszMessage, SQLINTEGER nNativeCode )
{
    MYODBCMessage *pmessage = new MYODBCMessage( nType, pszState, pszMessage, nNativeCode );
    emit signalMessage( pmessage );
    return pmessage;
}


#ifdef Q_WS_WIN
bool MYQTODBCEnvironment::doManageDataSources( QWidget *pwidget )
{
/* PAH - SQLManageDataSources() is a much better solution on MS Windows than this...
    QProcess  * pprocess = new QProcess( this );

    pprocess->addArgument( "ODBCAD32.exe" );
    if ( pprocess->start() ) 
        return;

    delete pprocess;
    pprocess = new QProcess( this );

    pprocess->addArgument( "c:\\windows\\system\\ODBCAD32.exe" );
    if ( pprocess->start() ) 
        return;

    delete pprocess;
    pprocess = new QProcess( this );

    pprocess->addArgument( "c:\\windows\\system32\\ODBCAD32.exe" );
    if ( pprocess->start() ) 
        return;

    delete pprocess;
    pprocess = new QProcess( this );

    pprocess->addArgument( "c:\\winnt\\system32\\ODBCAD32.exe" );
    if ( pprocess->start() ) 
        return;

    delete pprocess;

    QMessageBox::warning( 0, "CodeByDesign",  "Failed to execute ODBCAD32.exe from any of the usual places.\nIt would be cool if you were to report any additional locations I should have looked to pharvey@codebydesign.com.\nIn the meantime you can run the ODBC Administrator from the control panel.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
*/
	return SQLManageDataSources( (HWND)(pwidget->winId()) );
}
#else
#ifdef Q_WS_MACX
bool MYQTODBCEnvironment::doManageDataSources( QWidget * )
{
    QProcess  * pprocess = new QProcess( this );

    pprocess->addArgument( "open" );
    pprocess->addArgument( "/Applications/Utilities/ODBC Administrator.app" ); // OSX Jaguar now has a standard ODBC Administrator
    if ( pprocess->start() ) 
    {
//        QMessageBox::warning( 0, APPNAME,  "ODBCConfig has been started and is listed in the Dock. You may have to click on it to bring it to the front." );
        return true;
    }
    
    delete pprocess;

    QMessageBox::warning( 0, "MYQTODBCCls",  "Failed to execute the ODBC Administrator.\nIt should have been in /Applications/Utilities.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );

	return false;
}
#else
bool MYQTODBCEnvironment::doManageDataSources( QWidget *pwidgetParent )
{
    /*
     * Method 1 
     *
     * Execute a seperate process.
     *
     */
    QProcess  * pprocess = new QProcess( this );
    pprocess->addArgument( "ODBCConfig" );
    if ( pprocess->start() ) 
        return true;

    delete pprocess;
    pprocess = new QProcess( this );

    pprocess->addArgument( "gODBCConfig" );
    if ( pprocess->start() ) 
        return true;

    delete pprocess;

    QMessageBox::warning( pwidgetParent, "MYQTODBCCls",  "Failed to execute an ODBC Config tool - this sucks but it can easily be corrected.\nSimply ensure that ODBCConfig or gODBCConfig is in your path.\nThese programs are availible on www.sourceforge.com (search for unixODBC).", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );

    /*
     * Method 2
     *
     * Invoke using a Qt dialog.
     *
     * 1. Assumes we are using unixODBC.
     * 2. Assumes that unixODBC was built with GUI support.
     * 3. Assumes that there are no conflicts between the Qt lib used here
     *    and the one used to build unixODBC.
     */
/*
    ODBCINSTWND odbcinstwnd;
    
    odbcinstwnd.szGUI[0]    = 'Q';
    odbcinstwnd.szGUI[1]    = 't';
    odbcinstwnd.szGUI[2]    = '\0';
    odbcinstwnd.hWnd        = pwidget;

	return SQLManageDataSources( (HWND)(&odbcinstwnd) );
*/

	return false;
}
#endif
#endif


