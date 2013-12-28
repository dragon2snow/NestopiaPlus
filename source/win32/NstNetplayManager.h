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

#ifndef NST_NETPLAYMANAGER_H
#define NST_NETPLAYMANAGER_H

#include "NstManager.h"
#include "../NstKaillera.h"
#include "../NstNes.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class NETPLAYMANAGER : private MANAGER
{
public:

	NETPLAYMANAGER();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);
	VOID Start   ();

	inline VOID Update(NES::IO::INPUT& input)
	{ kaillera.Update( input ); }

	inline BOOL IsSupported() const
	{ return kaillera.IsSupported(); }

	inline BOOL IsConnected() const
	{ return kaillera.IsConnected(); }

	inline KAILLERA& Kaillera()
	{ return kaillera; }

private:

	enum
	{
		INTERVAL_DEF  = 0,
		INTERVAL_MAX  = 8,
		START_KAILLERA = 0xABC
	};

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	VOID UpdateDialog   (HWND);
	VOID UpdateSettings (HWND);
	VOID OnDefault      (HWND);
	VOID OnAdd          (HWND);
	VOID OnRemove       (HWND);
	VOID OnClearAll     (HWND);
	VOID OnDropFiles    (HWND,WPARAM);

	typedef PDXARRAY<PDXSTRING> GAMELIST;

	KAILLERA kaillera;
	BOOL UseDatabaseNames;
	BOOL PlayFullscreen;
	GAMELIST GameList;
};

#endif
