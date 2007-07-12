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

#include "MYODBCSetupDataSourceDialog.h"

/*!
    OSX

    We get an alarming warning from qt when we fail to provide
    the app name with a fully qualified absolute path. And the
    problem is that we do not have any real app path to give -
    so we make one up. Perhaps the real app name can be found
    but this seems to work. (busy busy so moveon.org)
*/
static char *pszAppName = "/myodbcinst";

BOOL MYODBCSetupDataSourceConfig( HWND hWnd, MYODBCUTIL_DATASOURCE *pDataSource )
{
    BOOL bReturn = FALSE;

    // No window handle - no gui - regardless of the fact that we may not use the window handle anyway :)
    if ( !hWnd )
        return FALSE;

    /*!
        \note   XP

                Qt4 eliminates setMainWidget(). This, or probably something else in qt4, has resulted
                in the app which dynamically loads the setup dll (ie; odbccad32.exe and myodbc3i.exe)
                to segfault on its way down. This does not happen when the setup code is simply compiled
                in (ie; myodbc3c.exe).

                Attempts to work-around this have failed so hopefully Trolls can advise. Meanwhile; the
                segfault does NOT affect the save of the config - mostly just the message is alarming.

                FYI: Just instantiating a QApplication can cause the problem so perhaps QApplication
                messes up the environment or is otherwise confused by our use.

                Linux

                Problem with conflicting QApplication between libmyodbc3S and ODBCConfig. myodbc3c (edit) 
                and even myodbc3i (create/edit) seem to work fine.
    */
#if QT_VERSION >= 0x040000
    if ( !QApplication::instance() )
    {
        int     argc    = 1;
        char *  argv[]  = { pszAppName, NULL };
        QApplication app( argc, argv );

//        MYODBCSetupDataSourceDialog *pdialogDataSource = new MYODBCSetupDataSourceDialog( 0, pDataSource );
//
//        if ( pdialogDataSource->exec() == QDialog::Accepted )
//            bReturn = TRUE; 

        MYODBCSetupDataSourceDialog dialogDataSource( NULL, pDataSource );

        if ( dialogDataSource.exec() == QDialog::Accepted )
            bReturn = TRUE; 

//        dialogDataSource.show();
//        app.exec();
//        delete pdialogDataSource;
    }
#else
    if ( !qApp )
    {
        int     argc    = 1;
        char *  argv[]  = { pszAppName, NULL };
        static  QApplication app( argc, argv );

        MYODBCSetupDataSourceDialog dialogDataSource( NULL, pDataSource );

        if ( dialogDataSource.exec() == QDialog::Accepted )
            bReturn = TRUE; 
    }
#endif
    else
    {
        // We get here when ODBCConfig calls setup and gets this dialog because ODBCConfig has a qApp. */
        MYODBCSetupDataSourceDialog dialogDataSource( NULL, pDataSource );

        if ( dialogDataSource.exec() == QDialog::Accepted )
            bReturn = TRUE; 
    }

    return bReturn;
}



