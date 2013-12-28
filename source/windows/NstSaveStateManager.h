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

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SAVESTATEMANAGER : public MANAGER
{
public:

	SAVESTATEMANAGER();

	VOID Create(CONFIGFILE* const);
	VOID Reset();
	
	VOID EnableFileExport(const BOOL export) 
	{ 
		if (ExportFile = export)
			FlushSlots();
	}

	VOID EnableFileImport(const BOOL import) 
	{ 
		if (ImportFile = import)
			CacheSlots();
	}

	PDX_NO_INLINE VOID SetFileName(const UINT,const PDXSTRING&);
	PDX_NO_INLINE VOID CacheSlots();
	PDX_NO_INLINE VOID FlushSlots() const;
	PDX_NO_INLINE PDXRESULT LoadState(UINT);
	PDX_NO_INLINE PDXRESULT SaveState(UINT);

	inline UINT GetLastSlot() const
	{ return LastSlot + 1; }

	inline const PDXSTRING& GetSlotFile(const UINT index) const
	{ return slots[IndexToSlot(index)].filename; }

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

	VOID UpdateDialog(HWND);
	VOID UpdateDialogTime(HWND,const WPARAM);
	VOID UpdateSettings(HWND);
	VOID OnAutoSave();
	VOID UpdateAutoSaveTimer();
	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);
	VOID OnBrowse(HWND);
	VOID OnClear(HWND);

	static VOID CALLBACK OnAutoSaveProc(HWND,UINT,UINT_PTR,DWORD);

	struct SLOT
	{
		VOID Reset()
		{
			filename.Clear();
			data.Clear();
		}

		PDXSTRING filename;
		PDXARRAY<CHAR> data;
	};

	BOOL ImportFile;
	BOOL ExportFile;
	UINT LastSlot;
	SLOT slots[MAX_SLOTS];

	struct AUTOSAVE
	{
		AUTOSAVE()
		:
		enabled (FALSE),
		msg     (FALSE),
		time    (0)
		{}

		BOOL enabled;
		PDXSTRING filename;
		BOOL msg;
		ULONG time;
	};

	AUTOSAVE AutoSave;
};

#endif
