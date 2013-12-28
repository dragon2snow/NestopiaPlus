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

#ifndef NST_MENU_H
#define NST_MENU_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class NSTMENU
{
public:

	NSTMENU(HMENU=NULL);
	NSTMENU(const UINT);

	enum FLAG
	{
		POS = MF_BYPOSITION,
		CMD = MF_BYCOMMAND
	};

	inline NSTMENU operator = (HMENU h)
	{ return (hMenu = h); }

	inline NSTMENU operator = (NSTMENU m)
	{ return (hMenu = m.hMenu); }

	inline BOOL IsSet() const
	{ return hMenu != NULL; }

	VOID Load(const UINT);
	VOID Destroy();

	VOID Insert(const UINT,const CHAR* const);
	VOID InsertSub(NSTMENU,const CHAR* const);

	BOOL Enable (const UINT,const BOOL=TRUE,const FLAG=CMD);
	BOOL Check  (const UINT,const BOOL=TRUE,const FLAG=CMD);

	inline BOOL ToggleCheck(const UINT i,const FLAG f=CMD)
	{ return Check( i, IsUnchecked( i, f ), f ); }

	inline BOOL Disable (const UINT i,const FLAG t=CMD) { return Enable ( i, FALSE, t ); }
	inline BOOL Uncheck (const UINT i,const FLAG t=CMD) { return Check  ( i, FALSE, t ); }

	BOOL IsEnabled (const UINT,const FLAG=CMD) const;
	BOOL IsChecked (const UINT,const FLAG=CMD) const;
	
	inline BOOL IsDisabled  (const UINT i,const FLAG t=CMD) const { return !IsEnabled( i, t ); }
	inline BOOL IsUnchecked (const UINT i,const FLAG t=CMD) const { return !IsChecked( i, t ); }

	NSTMENU GetSub(const UINT);

	VOID SetText(const UINT,const CHAR* const,const FLAG=CMD);
	CHAR* GetText(const UINT,PDXSTRING&,const FLAG=CMD) const;

	VOID Remove(const UINT,const FLAG=CMD);

	VOID Clear();
	VOID Attach(HWND);
	VOID Detach(HWND);
	VOID MakeModeless();

	INT GetHeight(HWND) const;

	inline operator HMENU()
	{ return hMenu; }

private:

	HMENU hMenu;
};

#endif
