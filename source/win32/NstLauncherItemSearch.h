///////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Nestopia.
// 
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef NST_LAUNCHER_ITEMSEARCH_H
#define NST_LAUNCHER_ITEMSEARCH_H

#include <CommDlg.h>

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class LAUNCHERITEMSEARCH
{
public:

	LAUNCHERITEMSEARCH();
	~LAUNCHERITEMSEARCH();

	VOID Open   (HWND);
	VOID Close  ();
	VOID OnMsg  (HWND,const LPARAM);
	VOID OnFind (HWND,const CHAR* const);

	inline BOOL IsDlgMessage(MSG& msg)
	{ return hFind && ::IsDialogMessage( hFind, &msg ); }

	inline BOOL IsDlgMessage(const UINT m) const
	{ return hFind && m == uMsg; }

	enum {STRING_BUFFER_LENGTH = 512};

private:

	HWND hFind;
	HWND hParent;
	const UINT uMsg;
	UINT uFlags;
	FINDREPLACE fr;
	CHAR StringBuffer[STRING_BUFFER_LENGTH+1];
};

#endif
