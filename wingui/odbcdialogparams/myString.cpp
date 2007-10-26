#include "stdafx.h"

#include "myString.h"

#include <WinSock2.h>
#include "../util/stringutil.h"

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

myString & concat( myString &left, const myString &right )
{
    if ( right == NULL )
        return left;

    size_t      len = myStrlen(right) + 1;
    myString    tmp = (myString) my_realloc((gptr)left,(myStrlen(left)+len)*sizeof(SQLWCHAR), MY_ALLOW_ZERO_PTR);

    
    if (tmp == NULL)
    {
        tmp = (myString) my_malloc((myStrlen(left)+len+1)*sizeof(SQLWCHAR), MYF(0));

        if ( tmp )
        {
            sqlwcharncpy(tmp,left,myStrlen(left)+1);
            x_free(left);
        }
    }

    if ( tmp != NULL )
    {
        left = tmp;
        sqlwcharncat2(left, right, &len );
    }

    //return left.length();
    return left;
}

size_t myStrlen( const myString & str )
{
    return sqlwcharlen( str );
}

myString & strAssign	( myString &dest, const wchar_t * src )
{
    if (src != NULL)
        return strAssign( dest, myString( src ) );
    else
    {
        if ( dest )
            *dest = 0;

        return dest;
    }
}

myString & strAssign( myString &dest, const myString & src )
{
    if ( dest )
        x_free(dest);
    dest = sqlwchardup( src, myStrlen( src ));

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
#endif