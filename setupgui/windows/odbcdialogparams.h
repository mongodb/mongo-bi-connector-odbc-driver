/*
  Copyright (C) 2007 MySQL AB

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

#ifndef __ODBCPARAMS_H__
#define __ODBCPARAMS_H__

#ifdef __cplusplus
extern "C" {
#endif

// four callbacks:
// called when [Help] pressed
typedef void HelpButtonPressedCallbackType(HWND dialog);
// called when [Test] pressed - show any messages to user here
//typedef const wchar_t * TestButtonPressedCallbackType(HWND dialog, DataSource* params);
// called when [OK] pressed - show errors here (if any) and return false to prevent dialog close
typedef BOOL AcceptParamsCallbackType(HWND dialog, DataSource* params);
// called when DataBase combobox Drops Down
//typedef const WCHAR** DatabaseNamesCallbackType(HWND dialog, DataSource* params);


#ifdef __cplusplus
}
#endif

#endif

