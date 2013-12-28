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

#ifndef NST_SAVESTATEMANAGER_H
#define NST_SAVESTATEMANAGER_H

#include "../paradox/PdxFile.h"
#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SAVESTATEMANAGER : public MANAGER
{
public:

	SAVESTATEMANAGER(const INT id) 
	: MANAGER(id) {}

	~SAVESTATEMANAGER()
	{ Destroy(NULL); }

	PDXRESULT LoadState(UINT);
	PDXRESULT SaveState(UINT);

	BOOL IsValidSlot(const UINT index) const
	{ return slots[IndexToSlot(index)].valid; }

	inline UINT GetLastSlot() const
	{ return LastSlot + 1; }

	enum
	{
		MAX_SLOTS = 9,
		NEXT_SLOT = 0,
		LAST_SLOT = 0
	};

private:

	inline UINT IndexToSlot(INT index) const
	{
		PDX_ASSERT(index <= MAX_SLOTS);

		if (index-- == LAST_SLOT)
			index = LastSlot;

		return index;
	}

	PDXRESULT Create  (PDXFILE* const);
	PDXRESULT Destroy (PDXFILE* const);

	VOID UpdateDialog(HWND);
	VOID UpdateDialogTime(HWND,const WPARAM);
	VOID UpdateSettings(HWND);
	VOID OnAutoSave();
	VOID UpdateAutoSaveTimer();
	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);
	VOID OnBrowse(HWND);
	VOID OnClear(HWND);
	VOID Reset();

	static VOID CALLBACK OnAutoSaveProc(HWND,UINT,UINT_PTR,DWORD);

	struct SLOT
	{
		PDXFILE file;
		BOOL valid;
	};

	UINT LastSlot;
	SLOT slots[MAX_SLOTS];

	struct AUTOSAVE
	{
		BOOL enabled;
		PDXSTRING name;
		BOOL msg;
		ULONG time;
		PDXFILE file;
	};

	AUTOSAVE AutoSave;
};

#endif
