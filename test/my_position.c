/*
  Copyright (C) 1997-2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "odbctap.h"

/** Test SQL_POSITION */
DECLARE_TEST(t_chunk)
{
    SQLRETURN rc;
    char      txt[100];
    SQLLEN    len;

    if (!driver_supports_setpos(hdbc))
      skip("driver doesn't support setpos");

    SQLExecDirect(hstmt,"drop table t_chunk",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_chunk(id int not null primary key, description varchar(50), txt text)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_chunk VALUES(1,'venu','Developer, MySQL AB')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_chunk VALUES(2,'monty','Michael Monty Widenius - main MySQL developer')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_chunk VALUES(3,'mysql','MySQL AB- Speed, Power and Precision')",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);    

    rc = SQLExecDirect(hstmt,"SELECT * from t_chunk",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 1);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 2, SQL_C_CHAR, txt, 100, &len);
    mystmt(hstmt,rc);
    printMessage("\ntxt:%s(%d)",txt,len);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_chunk)
END_TESTS


RUN_TESTS
