// mytest.cpp 

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

// defines

#define NUM_CONCUR_USER 100

// types

typedef struct
{
    SQLHENV   m_hEnvironment;
    SQLHDBC   m_hSession;
    pthread_t m_hThread;
} myconn_t;

// globals

myconn_t g_myconns[NUM_CONCUR_USER] = {0};

// functions

bool CreateConns(char *dsn, char *user, char *password)
{
    SQLRETURN rc;
    for (int i = 0; i < NUM_CONCUR_USER; i++)
    {
        // Allocate the environment handle.
        rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &(g_myconns[i].m_hEnvironment));
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLAllocHandle(environment) failed. [%d]\n", i);
            return false;
        }
        // Set the ODBC compliance level of the application.
        rc = SQLSetEnvAttr(g_myconns[i].m_hEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLSetEnvAttr(ODBC3) failed. [%d]\n", i);
            return false;
        }
        // Allocate the session handle.
        rc = SQLAllocHandle(SQL_HANDLE_DBC, g_myconns[i].m_hEnvironment, &(g_myconns[i].m_hSession));
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLAllocHandle(environment) failed. [%d]\n", i);
            return false;
        }
        // Connect to the database.
        rc = SQLConnect(g_myconns[i].m_hSession, (SQLCHAR *)dsn, strlen(dsn), (SQLCHAR *)user, strlen(user), (SQLCHAR *)password, strlen(password));
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLConnect() failed. [%d]\n", i);
            return false;
        }
        SQLHSTMT hstmt;
        rc = SQLAllocHandle(SQL_HANDLE_STMT, g_myconns[i].m_hSession, &hstmt);
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLAllocHandle(stmt) failed. [%d]\n", i);
            return false;
        }
#if 0
        // Initialize tables
        SQLExecDirect(hstmt, "DROP TABLE PickNum", SQL_NTS);
        rc = SQLExecDirect(hstmt, "CREATE TABLE PickNum ( \
                                          DEPARTMENT varchar(32) NOT NULL default '', \
                                          NEXT_ID int(11) default NULL, \
                                          PRIMARY KEY  (DEPARTMENT) \
                                          ) TYPE=InnoDB",SQL_NTS);
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLExecDirect(Create table failed. [%d]\n", i);
            return false;
        }
#endif       

        // Auto commit off
        rc = SQLSetConnectAttr(g_myconns[i].m_hSession, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_INTEGER);
        if (!SQL_SUCCEEDED(rc))
        {
            printf("SQLSetConnectAttr() failed. [%d]\n", i);
            return false;
        }
    }
    return true;
}

void PrintErr(SQLSMALLINT hType, SQLHANDLE handle)
{
    SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];     
    SQLCHAR szText[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLINTEGER nativeErr;
    SQLSMALLINT i = 1, len;
    SQLRETURN rc = SQL_ERROR;
    while (true)
    {
        rc = SQLGetDiagRec(hType, handle, i++, szState, &nativeErr, szText, SQL_MAX_MESSAGE_LENGTH - 1, &len);
        if (rc == SQL_ERROR || rc == SQL_INVALID_HANDLE || rc == SQL_NO_DATA)
        {
            break;
        }
        printf("%s\n%s\n", szState, szText);
    }
}

int AutoRecNum(SQLHDBC hConn, char *table_name)
{
    pthread_t hThread = pthread_self();
    SQLHSTMT hStmtGet;
    SQLRETURN rc;
    
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmtGet);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLAllocHandle() failed. Thread[%d]\n", hThread);
        return 0;
    }
    rc = SQLSetStmtAttr(hStmtGet, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLSetStmtAttr() failed. Thread[%d]\n", hThread);
        return 0;
    }
    int id = 0;
    SQLINTEGER ds;
    rc = SQLBindCol(hStmtGet, 1, SQL_C_SLONG, &id, sizeof(id), &ds);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLBindCol() failed. Thread[%d]\n", hThread);
        return 0;
    }
    char sql_get[1024];
    sprintf(sql_get, "SELECT NEXT_ID FROM PickNum WHERE DEPARTMENT='%s' FOR UPDATE", table_name);
    rc = SQLExecDirect(hStmtGet, reinterpret_cast < SQLCHAR*>(sql_get), SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLExecDirect(%s) failed. Thread[%d]\n", sql_get, hThread);
        PrintErr(SQL_HANDLE_STMT, hStmtGet);
        return 0;
    }
    SQLINTEGER rowc;
    rc = SQLRowCount(hStmtGet, &rowc);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLRowCount() failed. Thread[%d]\n", hThread);
        return 0;
    }
    rc = SQLFetchScroll(hStmtGet, SQL_FETCH_FIRST, 0);
    if (rc == SQL_NO_DATA)
    {
        return 0;
    }
    SQLHSTMT hStmtSet;
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmtSet);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLAllocHandle() failed. Thread[%d]\n", hThread);
        return 0;
    }
    rc = SQLSetStmtAttr(hStmtSet, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLSetStmtAttr() failed. Thread[%d]\n", hThread);
        return 0;
    }
    char sql_set[1024];
    sprintf(sql_set, "UPDATE PickNum SET NEXT_ID=%d WHERE DEPARTMENT='%s'", id + 1, table_name);
    rc = SQLExecDirect(hStmtSet, reinterpret_cast < SQLCHAR*>(sql_set), SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLExecDirect(%s) failed. Thread[%d]\n", sql_set, hThread);
        PrintErr(SQL_HANDLE_STMT, hStmtSet);
        return 0;
    }
    rc = SQLEndTran(SQL_HANDLE_DBC, hConn, SQL_COMMIT);	
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLEndTran() failed. Thread[%d]\n", hThread);
        PrintErr(SQL_HANDLE_DBC, hConn);
        return 0;
    }
    rc = SQLFreeHandle(SQL_HANDLE_STMT, hStmtGet);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLFreeHandle() failed. Thread[%d]\n", hThread);
        return id;
    }
    rc = SQLFreeHandle(SQL_HANDLE_STMT, hStmtSet);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLFreeHandle() failed. Thread[%d]\n", hThread);
        return id;
    }
    return id;
}

void *ThreadFunc(void *lpParam)
{
    if (lpParam == NULL)
    {
        return NULL;
    }
    
    myconn_t *pContext =(myconn_t*)lpParam;
    SQLHDBC hConn = pContext->m_hSession;
    char dept_name[128];
    switch (pContext->m_hThread % 3)
    {
        case 0:
            strcpy(dept_name, "Hardware");
            break;
        case 1:
            strcpy(dept_name, "Floor");
            break;
        default:
            strcpy(dept_name, "Paint");
            break;
    }
    int cust_id = 0;
    for (int run = 0; run < 1000; run++)
    {
        cust_id = AutoRecNum(hConn, dept_name);
        if (cust_id == 0)
        {
            // abort transactiom
            (void)SQLEndTran(SQL_HANDLE_DBC, hConn, SQL_ROLLBACK);	
            // stop using this connection if error
            break;
        }
        printf("Thread[%d]: %s now serving customer %d\n", pContext->m_hThread, dept_name, cust_id);
    }
    printf("Thread[%d]: ended\n", pContext->m_hThread);
    return NULL;
}

void ClearConns(void)
{
    SQLRETURN rc;
    for (int i = 0; i < NUM_CONCUR_USER; i++)
    {
        if (g_myconns[i].m_hSession != NULL)
        {
            rc = SQLDisconnect(g_myconns[i].m_hSession);
            if (!SQL_SUCCEEDED(rc))
            {
                printf("SQLDisconnect failed. [%d]\n", i); 
            }
            rc = SQLFreeHandle(SQL_HANDLE_DBC, g_myconns[i].m_hSession);
            if (!SQL_SUCCEEDED(rc))
            {
                printf("SQLFreeHandle(session) failed. [%d]\n", i); 
            }
        }
        if (g_myconns[i].m_hEnvironment != NULL)
        {
            rc = SQLFreeHandle(SQL_HANDLE_ENV, g_myconns[i].m_hEnvironment);
            if (!SQL_SUCCEEDED(rc))
            {
                printf("SQLFreeHandle(environment) failed. [%d]\n", i); 
            }
        }
    }
}

// main

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("Usage: %s DSN USER PASSWORD\n", argv[0]);
        return 1;
    }
    
    if (!CreateConns(argv[1], argv[2], argv[3]))
    {
        return 2;
    }
    
    for (int i = 0; i < NUM_CONCUR_USER; i++)
    {
        int status = pthread_create(&(g_myconns[i].m_hThread),
            NULL,
            ThreadFunc,
            &g_myconns[i]);
    }
    for (int i = 0; i < NUM_CONCUR_USER; i++)
    {
        if (g_myconns[i].m_hThread != 0)
        {
            pthread_join(g_myconns[i].m_hThread, NULL);
        }
    }
    
    ClearConns();
    return 0;
}

