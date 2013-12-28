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

#ifndef NST_FILEMANAGER_H
#define NST_FILEMANAGER_H

class ZIPFILE;
class PDXFILE;

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class FILEMANAGER : public MANAGER
{
public:

	FILEMANAGER(const INT id,const UINT chunk)
	: MANAGER( id, chunk ), ZipFile(NULL) {}

	PDXRESULT Load(INT=-1,const BOOL=TRUE);
	PDXRESULT LoadState();
	PDXRESULT SaveState();

	UINT NumRecentFiles() const
	{ return RecentFiles.Size(); }

	const PDXSTRING& GetRecentFile(const UINT i) const
	{ return RecentFiles[i]; }

	const PDXSTRING& GetRecentFile() const
	{ 
		PDX_ASSERT(RecentFiles.Size());
		return RecentFiles.Front();
	}

	inline BOOL UpdatedRecentFiles() const
	{ return UpdatedRecentFile; }

	VOID AddRecentFile(const CHAR* const);

	inline const PDXSTRING& GetRomPath() const
	{ return UseRomPathLast ? RomPathLast : RomPath; }

	inline const PDXSTRING& GetSavePath() const
	{ return UseSavePathRom ? GetRomPath() : SavePath; }

	inline const PDXSTRING& GetStatePath() const
	{ return UseStatePathLast ? StatePathLast : StatePath; }

	inline const PDXSTRING& GetIpsPath() const
	{ return UseIpsPathRom ? GetRomPath() : IpsPath; }

	inline UINT OpenZipFile(const CHAR* const title,const CHAR* const name,const PDXARRAY<PDXSTRING>& extensions,PDXFILE& file)
	{ return OpenZipFile( title, name, extensions, file, FALSE ); }

private:

	VOID UpdateContext();

	PDXRESULT Create  (PDXFILE* const);
	PDXRESULT Destroy (PDXFILE* const);

	enum
	{
		USE_ROM_PATH_LAST     = b0000001,
		USE_SAVE_PATH_ROM     = b0000010,
		USE_STATE_PATH_LAST   = b0000100,
		DISABLE_SAVERAM_WRITE = b0001000,
		USE_IPS_PATH_ROM      = b0010000,
		AUTO_APPLY_IPS        = b0100000
	};

	PDXRESULT LoadFile
	(
	    PDXFILE&,
       	const CHAR* const,
		const CHAR* const,
		const CHAR* const,
		const PDXARRAY<PDXSTRING>&,
		const BOOL=FALSE,
		const INT=-1
	);

	PDXRESULT SaveFile
	(
	    PDXFILE&,
       	const CHAR* const,
		const CHAR* const,
		const CHAR* const,
		const CHAR* const
	);

	typedef PDXARRAY<PDXSTRING> RECENTFILES;

	VOID Reset();
	VOID UpdateDialog();
	VOID UpdateSettings();
	BOOL SelectPath(PDXSTRING&,const CHAR* const);	
	UINT OpenZipFile(const CHAR* const,const CHAR* const,const PDXARRAY<PDXSTRING>&,PDXFILE&,const BOOL);

	static VOID ValidatePath(PDXSTRING&);
	static BOOL UpdateAccess(const PDXSTRING&);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	static BOOL CALLBACK CompressedFileDialogProc(HWND,UINT,WPARAM,LPARAM);

	HWND hDlg;

	PDXSTRING RomPath;
	PDXSTRING RomPathLast;
	BOOL      UseRomPathLast;
	PDXSTRING SavePath;
	BOOL      UseSavePathRom;
	BOOL      DisableSaveRamWrite;
	PDXSTRING StatePath;
	PDXSTRING StatePathLast;
	BOOL      UseStatePathLast;
	PDXSTRING IpsPath;
	BOOL      UseIpsPathRom;
	BOOL      AutoApplyIps;

	BOOL UpdatedRecentFile;

	INT CompressedFileIndex;
	ZIPFILE* ZipFile;

	enum {MAX_RECENT_FILES = 10};

	RECENTFILES RecentFiles;

	PDXSTRING ZipDialogTitle;
};

#endif
