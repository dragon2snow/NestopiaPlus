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

	FILEMANAGER(const INT id)
	: MANAGER(id), ZipFile(NULL) {}

	PDXRESULT Load(INT=-1,const BOOL=TRUE);
	PDXRESULT LoadNSP(INT=-1,const BOOL=TRUE);
	PDXRESULT LoadNST();
	PDXRESULT SaveNSP();
	PDXRESULT SaveNST();

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

	inline const PDXSTRING& GetRomPath()  const { return UseRomPathLast   ? RomPathLast   : RomPath;   }
	inline const PDXSTRING& GetSavePath() const { return UseSavePathRom   ? GetRomPath()  : SavePath;  }
	inline const PDXSTRING& GetNstPath()  const { return UseStatePathLast ? StatePathLast : StatePath; }
	inline const PDXSTRING& GetIpsPath()  const { return UseIpsPathRom    ? GetRomPath()  : IpsPath;   }
	inline const PDXSTRING& GetNspPath()  const { return UseNspPathLast   ? NspPathLast   : NspPath;   }

	inline UINT OpenZipFile(const CHAR* const title,const CHAR* const name,const PDXARRAY<PDXSTRING>& extensions,PDXFILE& file)
	{ return OpenZipFile( title, name, extensions, file, FALSE ); }

private:

	PDXRESULT LoadNesFile (PDXFILE&,const BOOL);

	PDXRESULT ApplyRom (const PDXSTRING&,PDXFILE&,PDXSTRING&);
	PDXRESULT ApplyNps (const PDXSTRING&,NES::IO::NSP::CONTEXT&,BOOL&) const;
	PDXRESULT ApplyIps (PDXFILE&,PDXSTRING&,BOOL&);
	PDXRESULT ApplyNst (PDXFILE&,PDXSTRING&,BOOL&) const;

	VOID ApplySav (const PDXSTRING&,PDXSTRING&);

	BOOL FindRom (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindIps (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindSav (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindNsp (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindNst (const PDXSTRING&,PDXSTRING&) const;

	VOID UpdateContext();

	PDXRESULT Create  (CONFIGFILE* const);
	PDXRESULT Destroy (CONFIGFILE* const);

	enum
	{
		USE_ROM_PATH_LAST     = 0x001,
		USE_SAVE_PATH_ROM     = 0x002,
		USE_STATE_PATH_LAST   = 0x004,
		DISABLE_SAVERAM_WRITE = 0x008,
		USE_IPS_PATH_ROM      = 0x010,
		AUTO_APPLY_IPS        = 0x020,
		USE_NSP_PATH_ROM      = 0x040,
		AUTO_APPLY_NSP        = 0x080,
		USE_NSP_PATH_LAST     = 0x100
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
	PDXSTRING NspPath;
	PDXSTRING NspPathLast;
	BOOL      UseNspPathRom;
	BOOL      UseNspPathLast;
	BOOL      AutoApplyNsp;

	PDXSTRING LastImageFile;
	PDXSTRING LastIpsFile;
	PDXSTRING LastSaveFile;

	BOOL UpdatedRecentFile;

	INT CompressedFileIndex;
	ZIPFILE* ZipFile;

	enum {MAX_RECENT_FILES = 10};

	RECENTFILES RecentFiles;
	PDXSTRING ZipDialogTitle;
};

#endif
