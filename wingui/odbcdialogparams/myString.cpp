#include "stdafx.h"

#include "myString.h"


/*myString & operator += ( myString & lhs, const wchar_t * rhs)
{
	lhs += myString(rhs);
	return lhs;
}*/

#ifndef _DO_NOT_USE_STRING_

myString & concat( myString &left, const myString &right )
{
	left += right;
	//return left.length();
	return left;
}

size_t myStrlen( myString & str )
{
	return str.length();
}

myString & strAssign	( myString &dest, const wchar_t * src )
{
    if (src != NULL)
	    return strAssign( dest, myString( src ) );
    else
    {
        dest= L"";
        return dest;
    }
}

myString & strAssign( myString &dest, const myString & src )
{
	dest = src;

	return dest;
}

void add2list( WCHAR ** & list, const wchar_t * newmember )
{
	size_t pos = listSize( (const WCHAR **) list);

	WCHAR** prev = list;

	list = (WCHAR **) realloc( list, (pos + 1 + 1)*sizeof(WCHAR*) );

	if ( list == NULL )
	{
		clearList( prev );
		return;
	}

	list[ pos + 1 ] = NULL;

	size_t len = wcslen(newmember);
	wchar_t *realnew = (wchar_t*)malloc(sizeof(wchar_t)*( len + 1 ) );

	wcsncpy( realnew, newmember, len + 1 );

	list[pos] = realnew;
}

void clearList( WCHAR ** & list )
{
	if ( ! list )
		return;

	for ( unsigned i = 0; list[ i ]; ++i )
	{
		free( list[i] );
	}

	free( list );

	list = NULL;
}

size_t	listSize( const WCHAR ** list )
{
	if ( ! list )
		return 0;/*InvalidSize*/;

	size_t size = 0;

	while ( list[size] ) ++size;

	return size;
}

#else
/*** will need to write for wchar_t* later when we will actually switch to it from wstring */
#endif