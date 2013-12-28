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

#ifndef NST_LAUNCHERFILESEARCH_H
#define NST_LAUNCHERFILESEARCH_H

#include "NstManager.h"
#include "../core/NstRomDatabase.h"
#include "../paradox/PdxSet.h"
#include "../paradox/PdxPair.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class LAUNCHERFILESEARCH : public MANAGER
{
public:

	enum
	{
		READ_NES      = 0x01,
		READ_UNF      = 0x02,
		READ_FDS      = 0x04,
		READ_NSF      = 0x08,
		READ_NSP      = 0x10,
		READ_ZIP      = 0x20,
		READ_ANY      = 0x40,
		PATH_NO_DUBLICATES = 0x80
	};

	typedef PDXARRAY<PDXPAIR<PDXSTRING,BOOL> > PATHS;

	LAUNCHERFILESEARCH();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);
	BOOL Refresh (const PATHS&,HWND);
	
	inline VOID SetFlags(const UINT f)
	{ flags = f; }

	struct ENTRY
	{
		ENTRY()
		: dBaseHandle(NULL) {}

		PDXSTRING file;
		PDXSTRING path;
		PDXSTRING name;
		PDXSTRING copyright;

		NES::ROMDATABASE::HANDLE dBaseHandle;

		enum TYPE
		{
			TYPE_NES = 0x01,
			TYPE_UNF = 0x02,
			TYPE_FDS = 0x04,
			TYPE_NSF = 0x08,
			TYPE_NSP = 0x10,
			TYPE_ZIP = 0x20,
			TYPE_ANY = TYPE_NES|TYPE_UNF|TYPE_FDS|TYPE_NSF|TYPE_NSP|TYPE_ZIP
		};

		U8 type;

		struct HEADER
		{
			HEADER()
			:
			pRomSize (0),
			cRomSize (0),
			wRamSize (0),
			mapper   (0),
			flags    (0)
			{}

			UINT pRomSize;
			UINT cRomSize;
			UINT wRamSize;
			U8 mapper;

			union
			{
				struct  
				{
					U8 flags;
				};

				struct  
				{
					UCHAR mirroring : 2;
					UCHAR battery   : 1;
					UCHAR trainer   : 1;
					UCHAR vs        : 1;
					UCHAR pal       : 1;
					UCHAR ntsc      : 1;
				};
			};
		};

		HEADER header;
	};

	inline TSIZE NumEntries() const
	{ return entries.Size(); }

	inline const ENTRY& GetEntry(const TSIZE i)
	{ return entries[i]; }

	BOOL AddEntry(const PDXSTRING&);
	VOID RemoveEntry(ENTRY* const);
	VOID RemoveAllEntries();

private:

	BOOL Load();
	BOOL Save() const;

	typedef PDXARRAY<ENTRY> ENTRIES;

	enum ABORT_SEARCH_EXCEPTION{ABORT_SEARCH};

	static DWORD WINAPI ThreadProc(LPVOID);
	static BOOL CALLBACK DlgProc(HWND,UINT,WPARAM,LPARAM);

	typedef PDXARRAY<CHAR> BUFFER;
	typedef PDXARRAY<PDXSTRING> EXTENSIONS;
	typedef PDXSET<PDXSTRING> SEARCHPATHS;
	typedef PDXSET<ULONG> CRCS;
	typedef PDXSET<const PDXSTRING> SAVEPATHS;

	static BOOL GetExtension(const CHAR* const,ULONG&);

	VOID ThreadRefresh();
	VOID RecursiveSearch(PDXSTRING&);

	VOID ReadPath (PDXSTRING&,const CHAR* const);
	VOID ReadFile (PDXSTRING&,const CHAR* const);
	
	BOOL IsDublicate();
	BOOL IsValid() const;

	BOOL ReadNes (const PDXSTRING&);
	BOOL ReadUnf (const PDXSTRING&);
	BOOL ReadFds (const PDXSTRING&);
	BOOL ReadNsf (const PDXSTRING&);
	BOOL ReadNsp (const PDXSTRING&);
	BOOL ReadZip (const PDXSTRING&);
	BOOL ReadAny (const PDXSTRING&);

	enum
	{
		RESULT_OK      = 0x00,
		RESULT_ABORTED = 0x01,
		RESULT_ERROR   = 0x02
	};

	HWND hDlg;
	BOOL stopped;
	HANDLE hThread;
	UINT flags;
	BOOL IncSubDirs;
	const PATHS* InputPaths;
	ENTRIES entries;
	BUFFER buffer;
	CRCS crcs;
	SEARCHPATHS SearchPaths;
	EXTENSIONS extensions;
};

#endif
