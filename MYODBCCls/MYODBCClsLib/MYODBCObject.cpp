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

#include "MYODBCObject.h"

MYODBCObject::MYODBCObject( MYODBCObject *pobjectParent )
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN parent=%p this=%p\n", __FILE__, __LINE__, pobjectParent, this );
#endif

    this->pobjectZParent = pobjectParent;
    pobjectZNext         = 0;
    pobjectZPrev         = 0;
    if ( pobjectZParent )
        pobjectZParent->getZChildren()->doAppend( this );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

MYODBCObject::~MYODBCObject()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN this=%p\n", __FILE__, __LINE__, this );
#endif

    // delete ALL children 
    // - this will recurse down the tree to delete ALL
    // - destructors MUST be able cleanup (i.e. cancel requests, disconnect, deallocate handles etc) before they return
    while ( objectlistZChildren.getFirst() )
    {
        delete objectlistZChildren.getFirst();
    }

    // extract ourself from our parents children list
    if ( pobjectZParent )
        pobjectZParent->getZChildren()->doRemove( this );

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}


