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

#ifndef NST_LAUNCHERCOLUMNS_H
#define NST_LAUNCHERCOLUMNS_H

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class LAUNCHERCOLUMNS : public MANAGER
{
public:

	LAUNCHERCOLUMNS();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	enum TYPE
	{
		TYPE_FILE,
		TYPE_SYSTEM,
		TYPE_MAPPER,
		TYPE_PROM,
		TYPE_CROM,
		TYPE_WRAM,
		TYPE_BATTERY,
		TYPE_TRAINER,
		TYPE_MIRRORING,
		TYPE_NAME,
		TYPE_COPYRIGHT,
		TYPE_CONDITION,
		TYPE_FOLDER,
		NUM_TYPES,
		NUM_DEFAULT_SELECTED_TYPES = 9,
		NUM_DEFAULT_AVAILABLE_TYPES = NUM_TYPES - NUM_DEFAULT_SELECTED_TYPES
	};

	typedef PDXARRAY<TYPE> TYPES;

	inline UINT NumSelectedTypes() const
	{ return selected.Size(); }

	inline TYPE GetType(const UINT i) const
	{ return selected[i]; }

	inline const CHAR* GetString(const UINT i) const
	{ return strings[GetType(i)]; }

	static const CHAR* ToString(const TYPE i)
	{ return strings[TYPE(i)]; }

	inline const TYPES& SelectedTypes() const
	{ return selected; }

private:

	VOID Reset  ();
	VOID Update (HWND,const BOOL);
	VOID Save   (HWND);

	static VOID Add(HWND,const UINT,const UINT);
	static VOID UpdateButtons(HWND);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	TYPES available;
	TYPES selected;

	static const CHAR* strings[NUM_TYPES]; 
};

#endif

