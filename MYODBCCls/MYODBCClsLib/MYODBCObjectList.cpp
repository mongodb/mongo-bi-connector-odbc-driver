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

#include "MYODBCObjectList.h"
#include "MYODBCObject.h"

/*!
    Constructs an Object List.
*/
MYODBCObjectList::MYODBCObjectList()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN this=%p\n", __FILE__, __LINE__, this );
#endif

    bAutoDelete     = false;
    pobjectFirst    = 0;
    pobjectLast     = 0;

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    Destructs an Object List.
*/
MYODBCObjectList::~MYODBCObjectList()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN this=%p\n", __FILE__, __LINE__, this );
#endif

    doClear();

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    Add pobject to start of list.
*/
void MYODBCObjectList::doPrepend( MYODBCObject *pobject )
{
    if ( !pobject )
        return;

    if ( pobjectFirst )
        pobjectFirst->pobjectZPrev = pobject;

    pobject->pobjectZPrev        = 0; 
    pobject->pobjectZNext        = pobjectFirst;
    
    if ( pobjectLast == pobjectFirst )
        pobjectLast = pobject;

    pobjectFirst = pobject;
}

/*!
    Add pobject to end of list.
*/
void MYODBCObjectList::doAppend( MYODBCObject *pobject )
{
    if ( !pobject )
        return;

    if ( pobjectLast )
        pobjectLast->pobjectZNext = pobject;

    pobject->pobjectZNext        = 0;
    pobject->pobjectZPrev        = pobjectLast;

    if ( pobjectFirst == pobjectLast )
        pobjectFirst = pobject;

    pobjectLast = pobject;
}

/*!
    Remove object from list.
*/
void MYODBCObjectList::doRemove( MYODBCObject *pobject )
{
    if ( !pobject )
        return;

    // extract
    if ( pobject->pobjectZNext )
        pobject->pobjectZNext->pobjectZPrev = pobject->pobjectZPrev;
    if ( pobject->pobjectZPrev )
        pobject->pobjectZPrev->pobjectZNext = pobject->pobjectZNext;
    // adjust first object as required
    if ( pobject == pobjectFirst )
        pobjectFirst = pobjectFirst->pobjectZNext;
    // adjust last object as required
    if ( pobject == pobjectLast )
        pobjectLast = pobjectLast->pobjectZPrev;

    // auto delete?
    if ( bAutoDelete )
        delete pobject;
}

/*!
    Removes all objects from the list using doRemove().
*/
void MYODBCObjectList::doClear()
{
    while ( pobjectFirst )
    {
        doRemove( pobjectFirst );
    }
}


