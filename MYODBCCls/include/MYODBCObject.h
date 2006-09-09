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

#ifndef ODBCOBJECT_H
#define ODBCOBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include <sqlext.h>

#ifdef USE_IODBC
#include <iodbcinst.h>
#else
#include <odbcinst.h>
#endif

#ifndef SQLLEN
#define SQLLEN SQLINTEGER
#endif

//
#include "MYODBCObjectList.h"

/*!
    The primary purpose of this is to maintain parent/child relationships
    and to facilitate a cleanup when parents are deleted.
    
    NOTE: Should add a mutex here to make thread safe!
*/

/** Base class for key classes in this library.
 *  Facilitates navigation and cleanup of the object hierarchy. 
 */
class MYODBCObject
{
    friend class MYODBCObjectList;
public:
    MYODBCObject( MYODBCObject *pobjectParent = 0 );
    virtual ~MYODBCObject();

    // SETTERS

    // GETTERS
    virtual const char *getZClassName() = 0; // simplistic  rtti
    MYODBCObject *        getZParent() { return pobjectZParent; }
    MYODBCObjectList *    getZChildren() { return &objectlistZChildren; }
    MYODBCObject *        getZPrev() { return pobjectZPrev; }
    MYODBCObject *        getZNext() { return pobjectZNext; }

    // DO'rs

protected:
    MYODBCObject *        pobjectZParent;      // Up
    MYODBCObjectList      objectlistZChildren; // Down (do not turn bAutoDelete on!)
    MYODBCObject *        pobjectZPrev;        // Left 
    MYODBCObject *        pobjectZNext;        // Right

    // SETTERS

    // GETTERS

    // DO'rs
};

#endif



