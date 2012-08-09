/*
  Copyright (c) 2010, 2011, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file  catalog.h
  @brief some definitions required for catalog functions
*/


/**
   enums for resultsets returned by catalog functions - we don't want
   magic numbers
*/

/* SQLColumns */
enum myodbcColumns {mycTABLE_CAT= 0,      mycTABLE_SCHEM,      mycTABLE_NAME,
              /*3*/ mycCOLUMN_NAME,       mycDATA_TYPE,        mycTYPE_NAME,
              /*6*/ mycCOLUMN_SIZE,       mycBUFFER_LENGTH,    mycDECIMAL_DIGITS,
              /*9*/ mycNUM_PREC_RADIX,    mycNULLABLE,         mycREMARKS,
              /*12*/mycCOLUMN_DEF,        mycSQL_DATA_TYPE,    mycSQL_DATETIME_SUB,
              /*15*/mycCHAR_OCTET_LENGTH, mycORDINAL_POSITION, mycIS_NULLABLE };

/* SQLProcedureColumns */
enum myodbcProcColumns {mypcPROCEDURE_CAT= 0, mypcPROCEDURE_SCHEM,  mypcPROCEDURE_NAME,
                  /*3*/ mypcCOLUMN_NAME,      mypcCOLUMN_TYPE,      mypcDATA_TYPE,
                  /*6*/ mypcTYPE_NAME,        mypcCOLUMN_SIZE,      mypcBUFFER_LENGTH,
                  /*9*/ mypcDECIMAL_DIGITS,   mypcNUM_PREC_RADIX,   mypcNULLABLE,
                  /*12*/mypcREMARKS,          mypcCOLUMN_DEF,       mypcSQL_DATA_TYPE,
                  /*15*/mypcSQL_DATETIME_SUB, mypcCHAR_OCTET_LENGTH,mypcORDINAL_POSITION, 
                  /*18*/mypcIS_NULLABLE };


/* Some common(for i_s/no_i_s) helper functions */
const char *my_next_token(const char *prev_token, 
                                char **token, 
                                char *data, 
                          const char chr);

SQLRETURN
create_empty_fake_resultset(STMT *stmt, MYSQL_ROW rowval, size_t rowsize,
                            MYSQL_FIELD *fields, uint fldcnt);

SQLRETURN
create_fake_resultset(STMT *stmt, MYSQL_ROW rowval, size_t rowsize,
                      my_ulonglong rowcnt, MYSQL_FIELD *fields, uint fldcnt);

my_bool server_has_i_s(DBC FAR *dbc);


/* no_i_s functions */
SQLRETURN
mysql_columns(STMT *hstmt, SQLCHAR *szCatalog, SQLSMALLINT cbCatalog,
             SQLCHAR *szSchema __attribute__((unused)),
             SQLSMALLINT cbSchema __attribute__((unused)),
             SQLCHAR *szTable, SQLSMALLINT cbTable,
             SQLCHAR *szColumn, SQLSMALLINT cbColumn);

SQLRETURN 
mysql_list_column_priv(SQLHSTMT hstmt,
                      SQLCHAR *catalog, SQLSMALLINT catalog_len,
                      SQLCHAR *schema __attribute__((unused)),
                      SQLSMALLINT schema_len __attribute__((unused)),
                      SQLCHAR *table, SQLSMALLINT table_len,
                      SQLCHAR *column, SQLSMALLINT column_len);

SQLRETURN
mysql_list_table_priv(SQLHSTMT hstmt,
                     SQLCHAR *catalog, SQLSMALLINT catalog_len,
                     SQLCHAR *schema __attribute__((unused)),
                     SQLSMALLINT schema_len __attribute__((unused)),
                     SQLCHAR *table, SQLSMALLINT table_len);

MYSQL_RES *mysql_table_status(STMT        *stmt,
                              SQLCHAR     *catalog,
                              SQLSMALLINT  catalog_length,
                              SQLCHAR     *table,
                              SQLSMALLINT  table_length,
                              my_bool      wildcard,
                              my_bool      show_tables,
                              my_bool      show_views);

MYSQL_RES *mysql_table_status_show(STMT        *stmt,
										               SQLCHAR     *catalog,
										               SQLSMALLINT  catalog_length,
										               SQLCHAR     *table,
										               SQLSMALLINT  table_length,
										               my_bool      wildcard);

SQLRETURN mysql_foreign_keys(SQLHSTMT hstmt,
                           SQLCHAR    *szPkCatalogName __attribute__((unused)),
                           SQLSMALLINT cbPkCatalogName __attribute__((unused)),
                           SQLCHAR    *szPkSchemaName __attribute__((unused)),
                           SQLSMALLINT cbPkSchemaName __attribute__((unused)),
                           SQLCHAR    *szPkTableName,
                           SQLSMALLINT cbPkTableName,
                           SQLCHAR    *szFkCatalogName,
                           SQLSMALLINT cbFkCatalogName,
                           SQLCHAR    *szFkSchemaName __attribute__((unused)),
                           SQLSMALLINT cbFkSchemaName __attribute__((unused)),
                           SQLCHAR    *szFkTableName,
                           SQLSMALLINT cbFkTableName);


SQLRETURN
mysql_primary_keys(SQLHSTMT hstmt,
                 SQLCHAR *catalog, SQLSMALLINT catalog_len,
                 SQLCHAR *schema __attribute__((unused)),
                 SQLSMALLINT schema_len __attribute__((unused)),
                 SQLCHAR *table, SQLSMALLINT table_len);


SQLRETURN
mysql_procedure_columns(SQLHSTMT hstmt,
                    SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
                    SQLCHAR *szSchemaName __attribute__((unused)),
                    SQLSMALLINT cbSchemaName __attribute__((unused)),
                    SQLCHAR *szProcName, SQLSMALLINT cbProcName,
                    SQLCHAR *szColumnName, SQLSMALLINT cbColumnName);


SQLRETURN
mysql_special_columns(SQLHSTMT hstmt, SQLUSMALLINT fColType,
                      SQLCHAR *szTableQualifier, SQLSMALLINT cbTableQualifier,
                      SQLCHAR *szTableOwner __attribute__((unused)),
                      SQLSMALLINT cbTableOwner __attribute__((unused)),
                      SQLCHAR *szTableName, SQLSMALLINT cbTableName,
                      SQLUSMALLINT fScope __attribute__((unused)),
                      SQLUSMALLINT fNullable __attribute__((unused)));

/*
  @purpose : retrieves a list of statistics about a single table and the
       indexes associated with the table. The driver returns the
       information as a result set.
*/

SQLRETURN
mysql_statistics(SQLHSTMT hstmt,
                SQLCHAR *catalog, SQLSMALLINT catalog_len,
                SQLCHAR *schema __attribute__((unused)),
                SQLSMALLINT schema_len __attribute__((unused)),
                SQLCHAR *table, SQLSMALLINT table_len,
                SQLUSMALLINT fUnique,
                SQLUSMALLINT fAccuracy __attribute__((unused)));

SQLRETURN
mysql_tables(SQLHSTMT hstmt,
             SQLCHAR *catalog, SQLSMALLINT catalog_len,
             SQLCHAR *schema, SQLSMALLINT schema_len,
             SQLCHAR *table, SQLSMALLINT table_len,
             SQLCHAR *type, SQLSMALLINT type_len);
