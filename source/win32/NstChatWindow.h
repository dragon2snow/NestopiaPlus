////////////////////////////////////////////////////////////////////////////////////////
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

#ifndef NST_CHATWINDOW_H
#define NST_CHATWINDOW_H

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class CHATWINDOW : private MANAGER
{
public:

	CHATWINDOW();
	~CHATWINDOW();

	typedef VOID (*CHATSENDCALLBACK)(PDXSTRING&);

	VOID Open(CHATSENDCALLBACK);
	VOID Close();

	inline HWND GetHWnd()
	{ return hDlg; }

	inline BOOL IsActive() const
	{ return hDlg != NULL; }

	inline BOOL IsDlgMsg(MSG& msg) const
	{ return hDlg && ::IsDialogMessage( hDlg, &msg ); }

private:

	VOID OnNotify(LPARAM);
	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	HWND hDlg;
	PDXSTRING ChatText;
	CHATSENDCALLBACK SendChatCallback;
};

#endif
