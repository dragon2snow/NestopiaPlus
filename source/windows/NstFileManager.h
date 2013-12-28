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

	FILEMANAGER();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	PDX_NO_INLINE PDXRESULT Load(INT=-1,const BOOL=TRUE);
	PDX_NO_INLINE PDXRESULT LoadNSP(INT=-1,const BOOL=TRUE);
	PDX_NO_INLINE PDXRESULT LoadNST();
	PDX_NO_INLINE PDXRESULT SaveNSP();
	PDX_NO_INLINE PDXRESULT SaveNST();

	UINT NumRecentFiles() const;

	const PDXSTRING& GetRecentFile(const UINT) const;
	const PDXSTRING& GetRecentFile() const;

	BOOL UpdatedRecentFiles() const;

	PDX_NO_INLINE VOID AddRecentFile(const CHAR* const);

	const PDXSTRING& GetRomPath()  const;
	const PDXSTRING& GetSavePath() const;
	const PDXSTRING& GetNstPath()  const;
	const PDXSTRING& GetIpsPath()  const;
	const PDXSTRING& GetNspPath()  const;

	UINT OpenZipFile
	(
    	const CHAR* const,
		const CHAR* const,
		const PDXARRAY<PDXSTRING>&,
		PDXFILE&
	);

	static PDX_NO_INLINE PDXRESULT GetExeFileName(PDXSTRING&);
	static PDX_NO_INLINE PDXRESULT GetExeFilePath(PDXSTRING&);
	static PDX_NO_INLINE PDXRESULT GetCurrentPath(PDXSTRING&);

private:

	PDX_NO_INLINE PDXRESULT LoadNesFile (PDXFILE&,const BOOL);
	
	PDX_NO_INLINE PDXRESULT ApplyRom (const PDXSTRING&,PDXFILE&,PDXSTRING&);
	PDX_NO_INLINE PDXRESULT ApplyNsp (const PDXSTRING&,NES::IO::NSP::CONTEXT&,BOOL&) const;
	PDX_NO_INLINE PDXRESULT ApplyIps (PDXFILE&,PDXSTRING&,BOOL&);
	PDX_NO_INLINE PDXRESULT ApplyNst (PDXFILE&,PDXSTRING&,BOOL&) const;
	
	VOID PDX_NO_INLINE ApplySav (const PDXSTRING&,PDXSTRING&);

	BOOL FindRom (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindIps (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindSav (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindNsp (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindNst (const PDXSTRING&,PDXSTRING&) const;
	BOOL FindSlt (const UINT,const PDXSTRING&,PDXSTRING&) const;

	PDX_NO_INLINE VOID UpdateContext();

	PDX_NO_INLINE PDXRESULT LoadFile
	(
	    PDXFILE&,
       	const CHAR* const,
		const CHAR* const,
		const CHAR* const,
		const PDXARRAY<PDXSTRING>&,
		const BOOL=FALSE,
		const INT=-1
	);

	PDX_NO_INLINE PDXRESULT SaveFile
	(
	    PDXFILE&,
       	const CHAR* const,
		const CHAR* const,
		const CHAR* const,
		const CHAR* const
	);

	typedef PDXARRAY<PDXSTRING> RECENTFILES;

	PDX_NO_INLINE VOID Reset();
	PDX_NO_INLINE VOID UpdateDialog();
	PDX_NO_INLINE BOOL SelectPath(PDXSTRING&,const CHAR* const);	
	
	PDX_NO_INLINE UINT OpenZipFile
	(
       	const CHAR* const,
		const CHAR* const,
		const PDXARRAY<PDXSTRING>&,
		PDXFILE&,
		const BOOL
	);

	VOID UpdateSettings();

	static PDX_NO_INLINE VOID ValidatePath(PDXSTRING&);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	static BOOL CALLBACK CompressedFileDialogProc(HWND,UINT,WPARAM,LPARAM);

	HWND hDlg;

	PDXSTRING RomPath;
	PDXSTRING RomPathLast;
	BOOL      UseRomPathLast;
	PDXSTRING SavePath;
	BOOL      UseSavePathRom;
	BOOL      DisableSaveRamWrite;
	PDXSTRING NstPath;
	PDXSTRING NstPathLast;
	BOOL      UseNstPathRom;
	BOOL      UseNstPathLast;
	BOOL      AutoImportNst;
	BOOL      AutoExportNst;
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

#include "NstFileManager.inl"

#endif
