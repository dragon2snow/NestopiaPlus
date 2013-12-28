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

#ifndef NST_LAUNCHERPATHS_H
#define NST_LAUNCHERPATHS_H

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class LAUNCHERPATHS : public MANAGER
{
public:

	LAUNCHERPATHS();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	struct FOLDER
	{
		FOLDER()
		: IncSubFolders(FALSE) {}

		PDXSTRING folder;
		BOOL IncSubFolders;
	};

	typedef PDXARRAY<FOLDER> FOLDERS;

	inline const FOLDERS& Folders() const
	{ return folders; }

	inline BOOL IncludeNes()   const { return flags & PATH_INC_NES;       }    
	inline BOOL IncludeUnf()   const { return flags & PATH_INC_UNF;       }
	inline BOOL IncludeFds()   const { return flags & PATH_INC_FDS;       }
	inline BOOL IncludeNsf()   const { return flags & PATH_INC_NSF;       }
	inline BOOL IncludeNsp()   const { return flags & PATH_INC_NSP;       }
	inline BOOL IncludeZip()   const { return flags & PATH_INC_ZIP;       }
	inline BOOL SearchAll()    const { return flags & PATH_SEARCH_ALL;    }
	inline BOOL NoDublicates() const { return flags & PATH_NO_DUBLICATES; }

private:

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);
	
	VOID Update (HWND);
	VOID Save   (HWND);

	static VOID OnAdd    (HWND);
	static VOID OnRemove (HWND);
	static VOID OnClear  (HWND);
	
	PDX_NO_INLINE static VOID UpdateButtons (HWND);
						
	enum
	{
		PATH_INC_NES       = 0x01,
		PATH_INC_UNF       = 0x02,
		PATH_INC_FDS       = 0x04,
		PATH_INC_NSF       = 0x08,
		PATH_INC_NSP       = 0x10,
		PATH_INC_ZIP       = 0x20,
		PATH_INC_ALL       = PATH_INC_NES|PATH_INC_UNF|PATH_INC_FDS|PATH_INC_NSF|PATH_INC_NSP|PATH_INC_ZIP,
		PATH_NO_DUBLICATES = 0x40,
		PATH_SEARCH_ALL    = 0x80 
	};

	UINT flags;
	FOLDERS folders;
};

#endif
