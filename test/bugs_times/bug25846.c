#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/*
  main routine
*/
int main(int argc, char *argv[])
{
    SQLHENV     henv;
    SQLHDBC     hdbc; 
    SQLHSTMT    hstmt;
    SQLRETURN   rc;
    SQLINTEGER  narg;

    SQLSMALLINT          column_count;
    SQLINTEGER           my_time_cb;
    SQLINTEGER           my_date_cb;
    SQL_TIMESTAMP_STRUCT my_time_ts;
    SQL_TIMESTAMP_STRUCT my_date_ts;
    
    struct tm *current_ts;
    time_t    sec_time;

    printMessageHeader();

    /*
      if connection string supplied through arguments, override
      the default one..
    */
    for (narg= 1; narg < argc; narg++)
    {
        if ( narg == 1 )
            mydsn= argv[1];
        else if ( narg == 2 )
            myuid= argv[2];
        else if ( narg == 3 )
            mypwd= argv[3];

    }   

    myconnect(&henv, &hdbc, &hstmt);  

    /* initialize data */
    SQLExecDirect(hstmt, "DROP TABLE my_time_tab", SQL_NTS);

    rc= SQLExecDirect(hstmt, "CREATE TABLE my_time_tab (ID INT PRIMARY KEY, \
                                                        my_time TIME, \
                                                        my_date DATE)", 
                                                        SQL_NTS);
    mystmt(hstmt, rc);

    /* Insert 15:45:30 into TIME column */
    rc= SQLExecDirect(hstmt, "INSERT INTO my_time_tab (ID, my_time, my_date) \
                             VALUES (0, '15:45:30', CURDATE())", 
                             SQL_NTS);
    mystmt(hstmt, rc);

    rc= SQLExecDirect(hstmt, "SELECT ID, my_time, my_date FROM my_time_tab",
                             SQL_NTS);
    mystmt(hstmt, rc);

    rc= SQLNumResultCols(hstmt, &column_count);
    mystmt(hstmt, rc);

    printMessage("Columns count: %d \n\n", column_count);

    /* Bind the TIMESTAMP buffer for TIME column */
    rc= SQLBindCol(hstmt, 2, SQL_C_TIMESTAMP, &my_time_ts, sizeof(my_time_ts), 
                   &my_time_cb);
    mystmt(hstmt, rc);

    /* Bind the TIMESTAMP buffer for DATE column */
    rc= SQLBindCol(hstmt, 3, SQL_C_TIMESTAMP, &my_date_ts, sizeof(my_date_ts),
                   &my_date_cb);
    mystmt(hstmt, rc);

    rc= SQLFetch(hstmt);
    mystmt(hstmt, rc);

    sec_time= time(NULL);
    current_ts= localtime(&sec_time);

    printMessage("CURRENT  DATE\n  YEAR: %d\n  MON: %d\n  DAY: %d \n\n", 
                 current_ts->tm_year + 1900,   /* year starts from 1900 */
                 current_ts->tm_mon + 1,       /* month starts from 0 */
                 current_ts->tm_mday);         /* days are ok */

    printMessage("TIME Column\n");
    printMessage("RETURNED DATE\n  YEAR: %d\n  MON: %d\n  DAY: %d \n",
                 my_time_ts.year, my_time_ts.month, my_time_ts.day);

    printMessage("RETURNED TIME\n  HOUR: %d\n  MIN: %d\n  SEC: %d \n\n",
                 my_time_ts.hour, my_time_ts.minute, my_time_ts.hour);

    printMessage("DATE Column\n");
    printMessage("RETURNED DATE\n  YEAR: %d\n  MON: %d\n  DAY: %d \n",
                 my_date_ts.year, my_date_ts.month, my_date_ts.day);

    myassert(my_time_ts.hour   == 15 && 
             my_time_ts.minute == 45 && 
             my_time_ts.second == 30);

    /* Most cases when we are not testin at 12:00 AM */
    if (my_time_ts.day == current_ts->tm_mday)
        myassert((current_ts->tm_year + 1900) == my_time_ts.year &&
                  my_time_ts.year             == my_date_ts.year && 
                  (current_ts->tm_mon + 1)    == my_time_ts.month &&
                  my_time_ts.month            == my_date_ts.month && 
                  current_ts->tm_mday         == my_time_ts.day &&
                  my_time_ts.day              == my_date_ts.day);
    else
        myassert((current_ts->tm_year + 1900) >= my_time_ts.year &&
                  my_time_ts.year             >= my_date_ts.year && 
                  (current_ts->tm_mon + 1)    >= my_time_ts.month &&
                  my_time_ts.month            >= my_date_ts.month &&
                  current_ts->tm_mday         >= my_time_ts.day &&
                  my_time_ts.day              >= my_date_ts.day);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    SQLExecDirect(hstmt, "DROP TABLE my_time_tab", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    mydisconnect(&henv,&hdbc,&hstmt);
    printMessageFooter( 1 );

    return 0;
}

