/*
  Copyright (C) 1995-2007 MySQL AB

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

/**
  @file  desc.c
  @brief Functions for handling descriptors.
*/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:                   *
 *   SQLSetDescField     (ISO 92)                                          *
 *   SQLGetDescField     (ISO 92)                                          *
 *   SQLCopyDesc         (ISO 92)                                          *
 ****************************************************************************/

#include "driver.h"

/* Utility macros for defining descriptor fields */
#define HDR_FLD(field, perm, type) \
  static desc_field HDR_##field= \
    {(perm), (type), DESC_HDR, offsetof(DESC, field)}
    /* parens around field in offsetof() confuse GCC */

#define REC_FLD(field, perm, type) \
  static desc_field REC_##field= \
    {(perm), (type), DESC_REC, offsetof(DESCREC, field)}


/*
 * Allocate a new descriptor.
 * Should be used to allocate 'implicit' descriptors on the statement
 * or 'explicit' user-requested descriptors.
 */
DESC *desc_alloc(STMT *stmt, SQLSMALLINT alloc_type,
                 desc_ref_type ref_type, desc_desc_type desc_type)
{
  DESC *desc= (DESC *)my_malloc(sizeof(DESC), MYF(MY_ZEROFILL));
  if (!desc)
    return NULL;
  /*
     We let the dynamic array handle the memory for the whole DESCREC,
     but in desc_get_rec we manually get a pointer to it. This avoids
     having to call set_dynamic after modifying the DESCREC.
  */
  if (my_init_dynamic_array(&desc->records, sizeof(DESCREC), 0, 0))
  {
    my_free((char *)desc, MYF(0));
    return NULL;
  }
  desc->desc_type= desc_type;
  desc->alloc_type= alloc_type;
  desc->ref_type= ref_type;
  desc->stmt= stmt;
  /* spec-defined defaults/initialization */
  desc->array_size= 1;
  desc->array_status_ptr= NULL;
  desc->bind_offset_ptr= NULL;
  desc->bind_type= SQL_BIND_BY_COLUMN;
  desc->count= 0;
  desc->rows_processed_ptr= NULL;
  desc->exp.stmts= NULL;
  return desc;
}


/*
  Free a descriptor.
*/
void desc_free(DESC *desc)
{
  assert(desc);
  if (IS_APD(desc))
    desc_free_paramdata(desc);
  delete_dynamic(&desc->records);
  x_free(desc);
}


/*
  Free any memory allocated for SQLPutData(). This is only useful
  for APDs.
*/
void desc_free_paramdata(DESC *desc)
{
  SQLLEN i;
  for (i= 0; i < desc->count; i++)
  {
    DESCREC *aprec= desc_get_rec(desc, i, FALSE);
    assert(aprec);
    if (aprec->par.alloced)
    {
      aprec->par.alloced= FALSE;
      my_free(aprec->par.value, MYF(0));
    }
  }
}


/*
 * Initialize APD
 */
void desc_rec_init_apd(DESCREC *rec)
{
  memset(rec, 0, sizeof(DESCREC));
  /* ODBC defaults */
  rec->concise_type= SQL_C_DEFAULT;
  rec->data_ptr= NULL;
  rec->indicator_ptr= NULL;
  rec->octet_length_ptr= NULL;
  rec->type= SQL_C_DEFAULT;

  /* internal */
  rec->par.alloced= FALSE;
  rec->par.value= NULL;
}


/*
 * Initialize IPD
 */
void desc_rec_init_ipd(DESCREC *rec)
{
  memset(rec, 0, sizeof(DESCREC));
  /* ODBC defaults */
  rec->fixed_prec_scale= SQL_TRUE;
  rec->local_type_name= (SQLCHAR *)"";
  rec->nullable= SQL_NULLABLE;
  rec->parameter_type= SQL_PARAM_INPUT;
  rec->type_name= (SQLCHAR *)"VARCHAR";
  rec->is_unsigned= SQL_FALSE;

  /* driver defaults */
  rec->name= (SQLCHAR *)"";
}


/*
 * Initialize ARD
 */
void desc_rec_init_ard(DESCREC *rec)
{
  memset(rec, 0, sizeof(DESCREC));
  /* ODBC defaults */
  rec->concise_type= SQL_C_DEFAULT;
  rec->data_ptr= NULL;
  rec->indicator_ptr= NULL;
  rec->octet_length_ptr= NULL;
  rec->type= SQL_C_DEFAULT;
}


/*
 * Initialize IRD
 */
void desc_rec_init_ird(DESCREC *rec)
{
  memset(rec, 0, sizeof(DESCREC));
  /* ODBC defaults */
  /* driver defaults */
  rec->auto_unique_value= SQL_FALSE;
  rec->case_sensitive= SQL_TRUE;
  rec->concise_type= SQL_VARCHAR;
  rec->display_size= 100;/*?*/
  rec->fixed_prec_scale= SQL_TRUE;
  rec->length= 100;/*?*/
  rec->nullable= SQL_NULLABLE_UNKNOWN;
  rec->type= SQL_VARCHAR;
  rec->type_name= (SQLCHAR *)"VARCHAR";
  rec->unnamed= SQL_UNNAMED;
  rec->is_unsigned= SQL_FALSE;
}


/*
 * Get a record from the descriptor.
 *
 * @param desc Descriptor
 * @param recnum 0-based record number
 * @param expand Whether to expand the descriptor to include the given
 *               recnum.
 * @return The requested record of NULL if it doesn't exist
 *         (and isn't created).
 */
DESCREC *desc_get_rec(DESC *desc, int recnum, my_bool expand)
{
  DESCREC *rec= NULL;
  int i;
  assert(recnum >= 0);
  /* expand if needed */
  if (expand)
  {
    for (i= desc->count; expand && i <= recnum; ++i)
    {
      /* we might have used records lying around from before if
       * SQLFreeStmt() was called with SQL_UNBIND or SQL_FREE_PARAMS
       */
      if (i < desc->records.elements)
      {
        rec= ((DESCREC *)desc->records.buffer) + recnum;
      }
      else
      {
        rec= (DESCREC *)alloc_dynamic(&desc->records);
        if (!rec)
          return NULL;
      }
      memset(rec, 0, sizeof(DESCREC));
      desc->count++;

      /* record initialization */
      if (IS_APD(desc))
          desc_rec_init_apd(rec);
      else if (IS_IPD(desc))
          desc_rec_init_ipd(rec);
      else if (IS_ARD(desc))
          desc_rec_init_ard(rec);
      else if (IS_IRD(desc))
          desc_rec_init_ird(rec);
    }
  }
  if (recnum < desc->count)
    rec= ((DESCREC *)desc->records.buffer) + recnum;
  if (expand)
    assert(rec);
  return rec;
}


/*
 * Disassociate a statement from an explicitly allocated
 * descriptor.
 *
 * @param desc The descriptor
 * @param stmt The statement
 */
void desc_remove_stmt(DESC *desc, STMT *stmt)
{
  LIST *lstmt;

  if (desc->alloc_type != SQL_DESC_ALLOC_USER)
    return;

  for (lstmt= desc->exp.stmts; lstmt; lstmt= lstmt->next)
  {
    if (lstmt->data == stmt)
    {
      desc->exp.stmts= list_delete(desc->exp.stmts, lstmt);
      return;
    }
  }

  assert(!"Statement was not associated with descriptor");
}


/*
 * Apply the actual value to the descriptor field.
 *
 * @param dest Pointer to descriptor field to be set.
 * @param dest_type Type of descriptor field (same type constants as buflen).
 * @param src Value to be set.
 * @param buflen Length of value (as specified by SQLSetDescField).
 */
static void
apply_desc_val(void *dest, SQLSMALLINT dest_type, void *src, SQLINTEGER buflen)
{
  switch (buflen)
  {
  case SQL_IS_SMALLINT:
  case SQL_IS_INTEGER:
  case SQL_IS_LEN:
    if (dest_type == SQL_IS_SMALLINT)
      *(SQLSMALLINT *)dest= (SQLLEN)src;
    else if (dest_type == SQL_IS_USMALLINT)
      *(SQLUSMALLINT *)dest= (SQLLEN)src;
    else if (dest_type == SQL_IS_INTEGER)
      *(SQLINTEGER *)dest= (SQLLEN)src;
    else if (dest_type == SQL_IS_UINTEGER)
      *(SQLUINTEGER *)dest= (SQLLEN)src;
    else if (dest_type == SQL_IS_LEN)
      *(SQLLEN *)dest= (SQLLEN)src;
    else if (dest_type == SQL_IS_ULEN)
      *(SQLULEN *)dest= (SQLLEN)src;
    break;

  case SQL_IS_USMALLINT:
  case SQL_IS_UINTEGER:
  case SQL_IS_ULEN:
    if (dest_type == SQL_IS_SMALLINT)
      *(SQLSMALLINT *)dest= (SQLULEN)src;
    else if (dest_type == SQL_IS_USMALLINT)
      *(SQLUSMALLINT *)dest= (SQLULEN)src;
    else if (dest_type == SQL_IS_INTEGER)
      *(SQLINTEGER *)dest= (SQLULEN)src;
    else if (dest_type == SQL_IS_UINTEGER)
      *(SQLUINTEGER *)dest= (SQLULEN)src;
    else if (dest_type == SQL_IS_LEN)
      *(SQLLEN *)dest= (SQLULEN)src;
    else if (dest_type == SQL_IS_ULEN)
      *(SQLULEN *)dest= (SQLULEN)src;
    break;

  case SQL_IS_POINTER:
    *(SQLPOINTER *)dest= src;
    break;

  default:
    /* TODO it's an actual data length */
    /* free/malloc to the field and copy it */
    /* TODO .. check for 22001 - String data, right truncated
     * The FieldIdentifier argument was SQL_DESC_NAME,
     * and the BufferLength argument was a value larger
     * than SQL_MAX_IDENTIFIER_LEN.
     */
    break;
  }
}


/*
 * Get a descriptor field based on the constant.
 */
static desc_field *
getfield(SQLSMALLINT fldid)
{
  /* all field descriptions are immutable */
  /* See: SQLSetDescField() documentation
   * http://msdn2.microsoft.com/en-us/library/ms713560.aspx */
  HDR_FLD(alloc_type        , P_RI|P_RA          , SQL_IS_SMALLINT);
  HDR_FLD(array_size        , P_RA|P_WA          , SQL_IS_ULEN    );
  HDR_FLD(array_status_ptr  , P_RI|P_WI|P_RA|P_WA, SQL_IS_POINTER );
  HDR_FLD(bind_offset_ptr   , P_RA|P_WA          , SQL_IS_POINTER );
  HDR_FLD(bind_type         , P_RA|P_WA          , SQL_IS_INTEGER );
  HDR_FLD(count             , P_RI|P_WI|P_RA|P_WA, SQL_IS_LEN     );
  HDR_FLD(rows_processed_ptr, P_RI|P_WI          , SQL_IS_POINTER );

  REC_FLD(auto_unique_value, PR_RIR                     , SQL_IS_INTEGER);
  REC_FLD(base_column_name , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(base_table_name  , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(case_sensitive   , PR_RIR|PR_RIP              , SQL_IS_INTEGER);
  REC_FLD(catalog_name     , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(concise_type     , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_SMALLINT);
  REC_FLD(data_ptr         , PR_WAR|PR_WAP              , SQL_IS_POINTER);
  REC_FLD(display_size     , PR_RIR                     , SQL_IS_LEN);
  REC_FLD(fixed_prec_scale , PR_RIR|PR_RIP              , SQL_IS_SMALLINT);
  REC_FLD(indicator_ptr    , PR_WAR|PR_WAP              , SQL_IS_POINTER);
  REC_FLD(label            , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(length           , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_ULEN);
  REC_FLD(literal_prefix   , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(literal_suffix   , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(local_type_name  , PR_RIR|PR_RIP              , SQL_IS_POINTER);
  REC_FLD(name             , PR_RIR|PR_WIP              , SQL_IS_POINTER);
  REC_FLD(nullable         , PR_RIR|PR_RIP              , SQL_IS_SMALLINT);
  REC_FLD(num_prec_radix   , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_INTEGER);
  REC_FLD(octet_length     , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_LEN);
  REC_FLD(octet_length_ptr , PR_WAR|PR_WAP              , SQL_IS_POINTER);
  REC_FLD(parameter_type   , PR_WIP                     , SQL_IS_SMALLINT);
  REC_FLD(precision        , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_SMALLINT);
  REC_FLD(rowver           , PR_RIR|PR_RIP              , SQL_IS_SMALLINT);
  REC_FLD(scale            , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_SMALLINT);
  REC_FLD(schema_name      , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(searchable       , PR_RIR                     , SQL_IS_SMALLINT);
  REC_FLD(table_name       , PR_RIR                     , SQL_IS_POINTER);
  REC_FLD(type             , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_SMALLINT);
  REC_FLD(type_name        , PR_RIR|PR_RIP              , SQL_IS_POINTER);
  REC_FLD(unnamed          , PR_RIR|PR_WIP              , SQL_IS_SMALLINT);
  REC_FLD(is_unsigned      , PR_RIR|PR_RIP              , SQL_IS_SMALLINT);
  REC_FLD(updatable        , PR_RIR                     , SQL_IS_SMALLINT);
  REC_FLD(datetime_interval_code      , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_SMALLINT);
  REC_FLD(datetime_interval_precision , PR_WAR|PR_WAP|PR_RIR|PR_WIP, SQL_IS_INTEGER);

  /* match 'field' names above */
  switch(fldid)
  {
  case SQL_DESC_ALLOC_TYPE:
    return &HDR_alloc_type;
  case SQL_DESC_ARRAY_SIZE:
    return &HDR_array_size;
  case SQL_DESC_ARRAY_STATUS_PTR:
    return &HDR_array_status_ptr;
  case SQL_DESC_BIND_OFFSET_PTR:
    return &HDR_bind_offset_ptr;
  case SQL_DESC_BIND_TYPE:
    return &HDR_bind_type;
  case SQL_DESC_COUNT:
    return &HDR_count;
  case SQL_DESC_ROWS_PROCESSED_PTR:
    return &HDR_rows_processed_ptr;

  case SQL_DESC_AUTO_UNIQUE_VALUE:
    return &REC_auto_unique_value;
  case SQL_DESC_BASE_COLUMN_NAME:
    return &REC_base_column_name;
  case SQL_DESC_BASE_TABLE_NAME:
    return &REC_base_table_name;
  case SQL_DESC_CASE_SENSITIVE:
    return &REC_case_sensitive;
  case SQL_DESC_CATALOG_NAME:
    return &REC_catalog_name;
  case SQL_DESC_CONCISE_TYPE:
    return &REC_concise_type;
  case SQL_DESC_DATA_PTR:
    return &REC_data_ptr;
  case SQL_DESC_DISPLAY_SIZE:
    return &REC_display_size;
  case SQL_DESC_FIXED_PREC_SCALE:
    return &REC_fixed_prec_scale;
  case SQL_DESC_INDICATOR_PTR:
    return &REC_indicator_ptr;
  case SQL_DESC_LABEL:
    return &REC_label;
  case SQL_DESC_LENGTH:
    return &REC_length;
  case SQL_DESC_LITERAL_PREFIX:
    return &REC_literal_prefix;
  case SQL_DESC_LITERAL_SUFFIX:
    return &REC_literal_suffix;
  case SQL_DESC_LOCAL_TYPE_NAME:
    return &REC_local_type_name;
  case SQL_DESC_NAME:
    return &REC_name;
  case SQL_DESC_NULLABLE:
    return &REC_nullable;
  case SQL_DESC_NUM_PREC_RADIX:
    return &REC_num_prec_radix;
  case SQL_DESC_OCTET_LENGTH:
    return &REC_octet_length;
  case SQL_DESC_OCTET_LENGTH_PTR:
    return &REC_octet_length_ptr;
  case SQL_DESC_PARAMETER_TYPE:
    return &REC_parameter_type;
  case SQL_DESC_PRECISION:
    return &REC_precision;
  case SQL_DESC_ROWVER:
    return &REC_rowver;
  case SQL_DESC_SCALE:
    return &REC_scale;
  case SQL_DESC_SCHEMA_NAME:
    return &REC_schema_name;
  case SQL_DESC_SEARCHABLE:
    return &REC_searchable;
  case SQL_DESC_TABLE_NAME:
    return &REC_table_name;
  case SQL_DESC_TYPE:
    return &REC_type;
  case SQL_DESC_TYPE_NAME:
    return &REC_type_name;
  case SQL_DESC_UNNAMED:
    return &REC_unnamed;
  case SQL_DESC_UNSIGNED:
    return &REC_is_unsigned;
  case SQL_DESC_UPDATABLE:
    return &REC_updatable;
  case SQL_DESC_DATETIME_INTERVAL_CODE:
    return &REC_datetime_interval_code;
  case SQL_DESC_DATETIME_INTERVAL_PRECISION:
    return &REC_datetime_interval_precision;
  }
  return NULL;
}


/*
  @type    : ODBC 3.0 API
  @purpose : Get a field of a descriptor.
 */
SQLRETURN
MySQLGetDescField(SQLHDESC hdesc, SQLSMALLINT recnum, SQLSMALLINT fldid,
                  SQLPOINTER valptr, SQLINTEGER buflen, SQLINTEGER *outlen)
{
  desc_field *fld= getfield(fldid);
  DESC *desc= (DESC *)hdesc;
  void *src_struct;
  void *src;

  CLEAR_DESC_ERROR(desc);

  if (IS_IRD(desc) && desc->stmt->state < ST_PREPARED)
    /* TODO if it's prepared is the IRD still ok to access?
     * or must we pre-execute it */
    return set_desc_error(desc, "HY007",
              "Associated statement is not prepared",
              MYERR_S1007);

  if ((fld == NULL) ||
      /* header permissions check */
      (fld->loc == DESC_HDR &&
         (desc->ref_type == DESC_APP && (~fld->perms & P_RA)) ||
         (desc->ref_type == DESC_IMP && (~fld->perms & P_RI))))
  {
    return set_desc_error(desc, "HY091",
              "Invalid descriptor field identifier",
              MYERR_S1091);
  }
  else if (fld->loc == DESC_REC)
  {
    int perms= 0; /* needed perms to access */

    if (desc->ref_type == DESC_APP)
      perms= P_RA;
    else if (desc->ref_type == DESC_IMP)
      perms= P_RI;

    if (desc->desc_type == DESC_PARAM)
      perms= P_PAR(perms);
    else if (desc->desc_type == DESC_ROW)
      perms= P_ROW(perms);

    if ((~fld->perms & perms) == perms)
      return set_desc_error(desc, "HY091",
                "Invalid descriptor field identifier",
                MYERR_S1091);
  }

  /* get the src struct */
  if (fld->loc == DESC_HDR)
    src_struct= desc;
  else
  {
    if (recnum < 1 || recnum > desc->count)
      return set_desc_error(desc, "07009",
                "Invalid descriptor index",
                MYERR_07009);
    src_struct= desc_get_rec(desc, recnum - 1, FALSE);
    assert(src_struct);
  }

  src= ((char *)src_struct) + fld->offset;

  /* TODO checks when strings? */
  if ((fld->data_type == SQL_IS_POINTER && buflen != SQL_IS_POINTER) ||
      (fld->data_type != SQL_IS_POINTER && buflen == SQL_IS_POINTER))
    return set_desc_error(desc, "HY015",
                          "Invalid parameter type",
                          MYERR_S1015);

  switch (buflen)
  {
  case SQL_IS_SMALLINT:
    if (fld->data_type == SQL_IS_SMALLINT)
      *(SQLSMALLINT *)valptr= *(SQLSMALLINT *)src;
    else if (fld->data_type == SQL_IS_USMALLINT)
      *(SQLSMALLINT *)valptr= *(SQLUSMALLINT *)src;
    else if (fld->data_type == SQL_IS_INTEGER)
      *(SQLSMALLINT *)valptr= *(SQLINTEGER *)src;
    else if (fld->data_type == SQL_IS_UINTEGER)
      *(SQLSMALLINT *)valptr= *(SQLUINTEGER *)src;
    else if (fld->data_type == SQL_IS_LEN)
      *(SQLSMALLINT *)valptr= *(SQLLEN *)src;
    else if (fld->data_type == SQL_IS_ULEN)
      *(SQLSMALLINT *)valptr= *(SQLULEN *)src;
    break;

  case SQL_IS_USMALLINT:
    if (fld->data_type == SQL_IS_SMALLINT)
      *(SQLUSMALLINT *)valptr= *(SQLSMALLINT *)src;
    else if (fld->data_type == SQL_IS_USMALLINT)
      *(SQLUSMALLINT *)valptr= *(SQLUSMALLINT *)src;
    else if (fld->data_type == SQL_IS_INTEGER)
      *(SQLUSMALLINT *)valptr= *(SQLINTEGER *)src;
    else if (fld->data_type == SQL_IS_UINTEGER)
      *(SQLUSMALLINT *)valptr= *(SQLUINTEGER *)src;
    else if (fld->data_type == SQL_IS_LEN)
      *(SQLUSMALLINT *)valptr= *(SQLLEN *)src;
    else if (fld->data_type == SQL_IS_ULEN)
      *(SQLUSMALLINT *)valptr= *(SQLULEN *)src;
    break;

  case SQL_IS_INTEGER:
    if (fld->data_type == SQL_IS_SMALLINT)
      *(SQLINTEGER *)valptr= *(SQLSMALLINT *)src;
    else if (fld->data_type == SQL_IS_USMALLINT)
      *(SQLINTEGER *)valptr= *(SQLUSMALLINT *)src;
    else if (fld->data_type == SQL_IS_INTEGER)
      *(SQLINTEGER *)valptr= *(SQLINTEGER *)src;
    else if (fld->data_type == SQL_IS_UINTEGER)
      *(SQLINTEGER *)valptr= *(SQLUINTEGER *)src;
    else if (fld->data_type == SQL_IS_LEN)
      *(SQLINTEGER *)valptr= *(SQLLEN *)src;
    else if (fld->data_type == SQL_IS_ULEN)
      *(SQLINTEGER *)valptr= *(SQLULEN *)src;
    break;

  case SQL_IS_UINTEGER:
    if (fld->data_type == SQL_IS_SMALLINT)
      *(SQLUINTEGER *)valptr= *(SQLSMALLINT *)src;
    else if (fld->data_type == SQL_IS_USMALLINT)
      *(SQLUINTEGER *)valptr= *(SQLUSMALLINT *)src;
    else if (fld->data_type == SQL_IS_INTEGER)
      *(SQLUINTEGER *)valptr= *(SQLINTEGER *)src;
    else if (fld->data_type == SQL_IS_UINTEGER)
      *(SQLUINTEGER *)valptr= *(SQLUINTEGER *)src;
    else if (fld->data_type == SQL_IS_LEN)
      *(SQLUINTEGER *)valptr= *(SQLLEN *)src;
    else if (fld->data_type == SQL_IS_ULEN)
      *(SQLUINTEGER *)valptr= *(SQLULEN *)src;
    break;

  case SQL_IS_LEN:
    if (fld->data_type == SQL_IS_SMALLINT)
      *(SQLLEN *)valptr= *(SQLSMALLINT *)src;
    else if (fld->data_type == SQL_IS_USMALLINT)
      *(SQLLEN *)valptr= *(SQLUSMALLINT *)src;
    else if (fld->data_type == SQL_IS_INTEGER)
      *(SQLLEN *)valptr= *(SQLINTEGER *)src;
    else if (fld->data_type == SQL_IS_UINTEGER)
      *(SQLLEN *)valptr= *(SQLUINTEGER *)src;
    else if (fld->data_type == SQL_IS_LEN)
      *(SQLLEN *)valptr= *(SQLLEN *)src;
    else if (fld->data_type == SQL_IS_ULEN)
      *(SQLLEN *)valptr= *(SQLULEN *)src;
    break;

  case SQL_IS_ULEN:
    if (fld->data_type == SQL_IS_SMALLINT)
      *(SQLULEN *)valptr= *(SQLSMALLINT *)src;
    else if (fld->data_type == SQL_IS_USMALLINT)
      *(SQLULEN *)valptr= *(SQLUSMALLINT *)src;
    else if (fld->data_type == SQL_IS_INTEGER)
      *(SQLULEN *)valptr= *(SQLINTEGER *)src;
    else if (fld->data_type == SQL_IS_UINTEGER)
      *(SQLULEN *)valptr= *(SQLUINTEGER *)src;
    else if (fld->data_type == SQL_IS_LEN)
      *(SQLULEN *)valptr= *(SQLLEN *)src;
    else if (fld->data_type == SQL_IS_ULEN)
      *(SQLULEN *)valptr= *(SQLULEN *)src;
    break;

  case SQL_IS_POINTER:
    *(SQLPOINTER *)valptr= *(SQLPOINTER *)src;
    break;

  default:
    /* TODO it's an actual data length */
    /* free/malloc to the field and copy it, etc, etc */
    break;
  }

  return SQL_SUCCESS;
}


/*
  @type    : ODBC 3.0 API
  @purpose : Set a field of a descriptor.
 */
SQLRETURN
MySQLSetDescField(SQLHDESC hdesc, SQLSMALLINT recnum, SQLSMALLINT fldid,
                  SQLPOINTER val, SQLINTEGER buflen)
{
  desc_field *fld= getfield(fldid);
  DESC *desc= (DESC *)hdesc;
  void *dest_struct;
  void *dest;

  CLEAR_DESC_ERROR(desc);

  /* check for invalid IRD modification */
  if (IS_IRD(desc))
  {
    switch (fldid)
    {
    case SQL_DESC_ARRAY_STATUS_PTR:
    case SQL_DESC_ROWS_PROCESSED_PTR:
      break;
    default:
      return set_desc_error(desc, "HY016",
                            "Cannot modify an implementation row descriptor",
                            MYERR_S1016);
    }
  }

  if ((fld == NULL) ||
      /* header permissions check */
      (fld->loc == DESC_HDR &&
         ((desc->ref_type == DESC_APP && (~fld->perms & P_WA)) ||
          (desc->ref_type == DESC_IMP && (~fld->perms & P_WI)))))
  {
    return set_desc_error(desc, "HY091",
              "Invalid descriptor field identifier",
              MYERR_S1091);
  }
  else if (fld->loc == DESC_REC)
  {
    int perms= 0; /* needed perms to access */

    if (desc->ref_type == DESC_APP)
      perms= P_WA;
    else if (desc->ref_type == DESC_IMP)
      perms= P_WI;

    if (desc->desc_type == DESC_PARAM)
      perms= P_PAR(perms);
    else if (desc->desc_type == DESC_ROW)
      perms= P_ROW(perms);

    if ((~fld->perms & perms) == perms)
      return set_desc_error(desc, "HY091",
                "Invalid descriptor field identifier",
                MYERR_S1091);
  }

  /* get the dest struct */
  if (fld->loc == DESC_HDR)
    dest_struct= desc;
  else
  {
    if (recnum < 1)
      return set_desc_error(desc, "07009",
                "Invalid descriptor index",
                MYERR_07009);
    dest_struct= desc_get_rec(desc, recnum - 1, TRUE);
  }

  dest= ((char *)dest_struct) + fld->offset;

  /* some applications and even MSDN examples don't give a correct constant */
  if (buflen == 0)
    buflen= fld->data_type;

  /* TODO checks when strings? */
  if ((fld->data_type == SQL_IS_POINTER && buflen != SQL_IS_POINTER) ||
      (fld->data_type != SQL_IS_POINTER && buflen == SQL_IS_POINTER))
    return set_desc_error(desc, "HY015",
                          "Invalid parameter type",
                          MYERR_S1015);

  /* per-field checks/functionality */
  switch (fldid)
  {
  case SQL_DESC_COUNT:
    /* we just force the descriptor record count to expand */
    (void)desc_get_rec(desc, (SQLINTEGER)val - 1, TRUE);
    break;
  case SQL_DESC_NAME:
    /* We don't support named parameters, values stay as initialized */
    return set_desc_error(desc, "01S01",
                          "Option value changed",
                          MYERR_01S02);
  case SQL_DESC_UNNAMED:
    if (((SQLINTEGER)val) == SQL_NAMED)
      return set_desc_error(desc, "HY092",
                            "Invalid attribute/option identifier",
                            MYERR_S1092);
  }

  /* We have to unbind the value if not setting a buffer */
  switch (fldid)
  {
  case SQL_DESC_DATA_PTR:
  case SQL_DESC_OCTET_LENGTH_PTR:
  case SQL_DESC_INDICATOR_PTR:
    break;
  default:
    if (fld->loc == DESC_REC)
    {
      DESCREC *rec= (DESCREC *) dest_struct;
      rec->data_ptr= NULL;
      rec->octet_length_ptr= NULL;
      rec->indicator_ptr= NULL;
    }
  }

  apply_desc_val(dest, fld->data_type, val, buflen);

  /* post-set responsibilities */
  if ((IS_ARD(desc) || IS_APD(desc)) && fld->loc == DESC_REC)
  {
    DESCREC *rec= (DESCREC *) dest_struct;
    switch (fldid)
    {
    case SQL_DESC_TYPE:
      rec->concise_type= rec->type;
      rec->datetime_interval_code= 0;
      break;
    case SQL_DESC_CONCISE_TYPE:
      rec->type= get_type_from_concise_type(rec->concise_type);
      rec->datetime_interval_code=
        get_dticode_from_concise_type(rec->concise_type);
      break;
    case SQL_DESC_DATETIME_INTERVAL_CODE: /* TODO validation for this value? */
      /* SQL_DESC_TYPE has to have already been set */
      if (rec->type == SQL_DATETIME)
        rec->concise_type=
          get_concise_type_from_datetime_code(rec->datetime_interval_code);
      else
        rec->concise_type=
          get_concise_type_from_interval_code(rec->datetime_interval_code);
      break;
    }

    switch (fldid)
    {
    case SQL_DESC_TYPE:
    case SQL_DESC_CONCISE_TYPE:
      /* setup type specific defaults (TODO others besides SQL_C_NUMERIC)? */
      if (IS_ARD(desc) && rec->type == SQL_C_NUMERIC)
      {
        rec->precision= 38;
        rec->scale= 0;
      }
    }
  }

  /*
    Set "real_param_done" for parameters if all fields needed to bind
    a parameter are set.
  */
  if (IS_APD(desc) && val != NULL && fld->loc == DESC_REC)
  {
    DESCREC *rec= (DESCREC *) dest_struct;
    switch (fldid)
    {
    case SQL_DESC_DATA_PTR:
    case SQL_DESC_OCTET_LENGTH_PTR:
    case SQL_DESC_INDICATOR_PTR:
      rec->par.real_param_done= TRUE;
      break;
    }
  }

  return SQL_SUCCESS;
}


/*
  @type    : ODBC 3.0 API
  @purpose : Copy descriptor information from one descriptor to another.
             Errors are placed in the TargetDescHandle.
 */
SQLRETURN MySQLCopyDesc(SQLHDESC SourceDescHandle, SQLHDESC TargetDescHandle)
{
  DESC *src= (DESC *)SourceDescHandle;
  DESC *dest= (DESC *)TargetDescHandle;

  CLEAR_DESC_ERROR(dest);

  if (IS_IRD(dest))
    return set_desc_error(dest, "HY016",
                          "Cannot modify an implementation row descriptor",
                          MYERR_S1016);

  if (IS_IRD(src) && src->stmt->state < ST_PREPARED)
    return set_desc_error(dest, "HY007",
              "Associated statement is not prepared",
              MYERR_S1007);

  /* copy the records */
  delete_dynamic(&dest->records);
  if (my_init_dynamic_array(&dest->records, sizeof(DESCREC),
                            src->records.max_element,
                            src->records.alloc_increment))
  {
    return set_desc_error(dest, "HY001",
              "Memory allocation error",
              MYERR_S1001);
  }
  memcpy(dest->records.buffer, src->records.buffer,
         src->records.max_element * src->records.size_of_element);

  /* copy all fields */
  dest->array_size= src->array_size;
  dest->array_status_ptr= src->array_status_ptr;
  dest->bind_offset_ptr= src->bind_offset_ptr;
  dest->bind_type= src->bind_type;
  dest->count= src->count;
  dest->rows_processed_ptr= src->rows_processed_ptr;
  memcpy(&dest->error, &src->error, sizeof(MYERROR));

  /* TODO consistency check on target, if needed (apd) */

  return SQL_SUCCESS;
}


/*
 * Call SQLGetDescField in the "context" of a statement. This will copy
 * any error from the descriptor to the statement.
 */
SQLRETURN
stmt_SQLGetDescField(STMT *stmt, DESC *desc, SQLSMALLINT recnum,
                     SQLSMALLINT fldid, SQLPOINTER valptr,
                     SQLINTEGER buflen, SQLINTEGER *outlen)
{
  SQLRETURN rc;
  if ((rc= MySQLGetDescField((SQLHANDLE)desc, recnum, fldid,
                             valptr, buflen, outlen)) != SQL_SUCCESS)
    memcpy(&stmt->error, &desc->error, sizeof(MYERROR));
  return rc;
}


/*
 * Call SQLSetDescField in the "context" of a statement. This will copy
 * any error from the descriptor to the statement.
 */
SQLRETURN
stmt_SQLSetDescField(STMT *stmt, DESC *desc, SQLSMALLINT recnum,
                     SQLSMALLINT fldid, SQLPOINTER val, SQLINTEGER buflen)
{
  SQLRETURN rc;
  if ((rc= MySQLSetDescField((SQLHANDLE)desc, recnum, fldid,
                             val, buflen)) != SQL_SUCCESS)
    memcpy(&stmt->error, &desc->error, sizeof(MYERROR));
  return rc;
}


/*
 * Call SQLCopyDesc in the "context" of a statement. This will copy
 * any error from the descriptor to the statement.
 */
SQLRETURN stmt_SQLCopyDesc(STMT *stmt, DESC *src, DESC *dest)
{
  SQLRETURN rc;
  if ((rc= MySQLCopyDesc((SQLHANDLE)src, (SQLHANDLE)dest)) != SQL_SUCCESS)
    memcpy(&stmt->error, &dest->error, sizeof(MYERROR));
  return rc;
}


SQLRETURN SQL_API
SQLCopyDesc(SQLHDESC SourceDescHandle, SQLHDESC TargetDescHandle)
{
  return MySQLCopyDesc(SourceDescHandle, TargetDescHandle);
}

