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

#include <MYODBCUtil.h>
#include <MYODBCSetupDataSourceDialog.h>

#include <qapplication.h>
#include <qstring.h>

/*!
    Default args (in case missing on command-line).
*/
char *pszDataSourceName = "myodbc";

int main( int argc, char **argv )
{
    QApplication                    app( argc, argv  );
    int                             nResult;
    MYODBCUTIL_DATASOURCE *         pDataSource = MYODBCUtilAllocDataSource( MYODBCUTIL_DATASOURCE_MODE_DSN_EDIT );

    if ( argc > 1 )
        pszDataSourceName = argv[1];
    else
    {
        printf( "[%s][%d]INFO: $ myodbc3c [dsn-name]\n", __FILE__, __LINE__ );
        printf( "[%s][%d]INFO: defaulting dsn-name to %s\n", __FILE__, __LINE__, pszDataSourceName );
    }

    if ( !MYODBCUtilReadDataSource( pDataSource, pszDataSourceName ) )
        printf( "[%s][%d]INFO: %s does not seem to exist.\n", __FILE__, __LINE__, pszDataSourceName );

    MYODBCSetupDataSourceDialog dialog( NULL, pDataSource );

//    app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
//    pmainwidget->show();
    nResult = dialog.exec();

    if ( nResult == QDialog::Accepted )
    {
        MYODBCUtilWriteDataSource( pDataSource );
    }

    MYODBCUtilFreeDataSource( pDataSource );

    /* return code will get used by postinstall on OSX installer */
    /* for now - always return success */
    return 0;
}


