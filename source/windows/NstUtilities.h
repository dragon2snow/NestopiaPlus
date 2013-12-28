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

#ifndef NST_UTILITIES_H
#define NST_UTILITIES_H

#include "../paradox/PdxString.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

namespace UTILITIES
{
	CHAR* IdToString(const UINT,PDXSTRING&,const TSIZE=1024);

	inline PDXSTRING IdToString(const UINT id,const TSIZE length=1024)
	{
		PDXSTRING string;
		IdToString( id, string, length );
		return string;
	}

	VOID ToGUID   (const CHAR* const,GUID&);
	VOID FromGUID (const GUID&,PDXSTRING&);

	VOID GetCurrentDir (PDXSTRING&);
	VOID GetExeDir	   (PDXSTRING&);
	VOID GetExeName    (PDXSTRING&);

	BOOL PathExist (const PDXSTRING&);
	BOOL FileExist (const PDXSTRING&);

	BOOL ValidatePathName (PDXSTRING&);
	BOOL ValidatePath     (PDXSTRING&);
	BOOL ValidateFile     (PDXSTRING&);
	BOOL ValidateDir      (PDXSTRING&);

	inline PDXSTRING FromGUID(const GUID& guid)
	{
		PDXSTRING string;
		FromGUID( guid, string );
		return string;
	}

	inline GUID ToGUID(const CHAR* const string)
	{
		GUID guid;
		ToGUID( string, guid );
		return guid;
	}

	BOOL BrowseOpenFile
	(
		PDXSTRING&,
		HWND,
		const UINT,
		const CHAR* const,
		const CHAR* const=NULL
	);

	BOOL BrowseOpenFile
	(
       	PDXSTRING&,
		HWND,
		const CHAR* const,
		const CHAR* const,
		const CHAR* const=NULL
	);

	BOOL BrowseSaveFile
	(
		PDXSTRING&,
		HWND,
		const CHAR* const,
		const CHAR* const,
		const CHAR* const=NULL,
		const CHAR* const=NULL
	);

	BOOL BrowseSaveFile
	(
		PDXSTRING&,
		HWND,
		const UINT,
		const CHAR* const,
		const CHAR* const=NULL,
		const CHAR* const=NULL
	);
};

#endif
