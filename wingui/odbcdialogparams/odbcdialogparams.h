#ifndef __ODBCPARAMS_H__
#define __ODBCPARAMS_H__

#ifdef ODBCDIALOGPARAMS_EXPORTS
#define _ODBCPARAMDLL __declspec(dllexport)
#else
#define _ODBCPARAMDLL __declspec(dllimport)
#endif

#include "myString.h"

// procedure describes all items shown on the dialog
struct _ODBCPARAMDLL OdbcDialogParams
{
	myString		drvname;
	myString		drvdesc;
	myString		srvname;
	unsigned		port;
	myString		username;
	myString		password;
	myString		dbname;
	myString		socket;
	myString		sslkey;
	myString		sslcert;
	myString		sslca;
	myString		sslcapath;
	myString		sslcipher;
	myString		initstmt;
	myString		charset;

	// flags 1
	bool dont_optimize_column_width;
	bool return_matching_rows;
	bool allow_big_results;
	bool use_compressed_protocol;
	bool change_bigint_columns_to_int;
	bool safe;
	bool enable_auto_reconnect;
	bool enable_auto_increment_null_search;
	// flags 2
	bool dont_prompt_upon_connect;
	bool enable_dynamic_cursor;
	bool ignore_N_in_name_table;
	bool user_manager_cursor;
	bool dont_use_set_locale;
	bool pad_char_to_full_length;
	bool dont_cache_result;
	// flags 3
	bool return_table_names_for_SqlDesribeCol;
	bool ignore_space_after_function_names;
	bool force_use_of_named_pipes;
	bool no_catalog;
	bool read_options_from_mycnf;
	bool disable_transactions;
	bool force_use_of_forward_only_cursors;
	// debug
	bool save_queries;
};

// four callbacks:
// called when [Help] pressed
typedef void HelpButtonPressedCallbackType(HWND dialog);
// called when [Test] pressed - show any messages to user here
typedef const wchar_t * TestButtonPressedCallbackType(HWND dialog, OdbcDialogParams* params);
// called when [OK] pressed - show errors here (if any) and return false to prevent dialog close
typedef BOOL AcceptParamsCallbackType(HWND dialog, OdbcDialogParams* params);
// called when DataBase combobox Drops Down
typedef const WCHAR** DatabaseNamesCallbackType(HWND dialog, OdbcDialogParams* params);


// exported procedure (the one)
// returns TRUE if user pressed OK, FALSE - otherwise
int _ODBCPARAMDLL ShowOdbcParamsDialog(
    PWCHAR caption,                 /*[in] Dialog caption*/
	OdbcDialogParams* params,                  /*[inout] params */
	HWND ParentWnd = NULL,                     /* [in] could be NULL */
	HelpButtonPressedCallbackType* hcallback = NULL, /* [in] could be NULL */
	TestButtonPressedCallbackType* tcallback = NULL, /* [in] could be NULL */
	AcceptParamsCallbackType* acallback = NULL, /* [in] could be NULL */
	DatabaseNamesCallbackType* dcallback = NULL) /* [in] could be NULL */;



#endif