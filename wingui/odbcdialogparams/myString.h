#ifndef __MYSTRING_H__

#define __MYSTRING_H__

#ifndef _DO_NOT_USE_STRING_

/** Type definitions and includes if _use_ string class*/
#include <xstring>
typedef std::wstring		myString;

#else

/** Type definitions and includes if _do_not_ use string class*/

#include <string.h>
#typedef wchar_t*			myString;

#endif /*using wstring or wchar_t **/

/*myString & operator += ( myString & lhs, const wchar_t * rhs);*/
static const size_t InvalidSize = size_t(-1);

#ifndef _ODBCPARAMDLL
#ifdef ODBCDIALOGPARAMS_EXPORTS
#define _ODBCPARAMDLL __declspec(dllexport)
#else
#define _ODBCPARAMDLL __declspec(dllimport)
#endif
#endif

myString _ODBCPARAMDLL &	concat		( myString &left, const myString &right );

size_t _ODBCPARAMDLL		myStrlen	( myString & str );

myString _ODBCPARAMDLL &	strAssign	( myString &dest, const myString & src );
myString _ODBCPARAMDLL &	strAssign	( myString &dest, const wchar_t * src );

/*** List related functions */
void	_ODBCPARAMDLL		add2list	( WCHAR ** & list, const wchar_t * newmember );
void	_ODBCPARAMDLL		clearList	( WCHAR ** & list );
size_t	_ODBCPARAMDLL		listSize	( const WCHAR ** list );

#endif /* __MYSTRING__*/