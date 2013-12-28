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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <cstdio>
#include <Windows.h>
#include <Commdlg.h>
#include <ShlObj.h>
#include "NstUtilities.h"
#include "NstApplication.h"

namespace UTILITIES {

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE GetInstance()
{
	HINSTANCE hInstance = (APPLICATION::IsInstanced() ? application.GetInstance() : GetModuleHandle(NULL));

	if (!hInstance)
		throw ("GetModuleHandle() failed!");

	return hInstance;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

CHAR* IdToString(const UINT id,PDXSTRING& string,const TSIZE length)
{
	PDX_ASSERT( id && length );

	string.Buffer().Resize( length + 1 );
	string.Buffer().Front() = '\0';

	::LoadString( GetInstance(), id, string.Begin(), length );

	string.Validate();	

	return string.Begin();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GetCurrentDir(PDXSTRING& dir)
{
	dir.Buffer().Resize( MAX_PATH + 1 );
	dir.Buffer().Front() = '\0';

	::GetCurrentDirectory( MAX_PATH, dir.Begin() );

	dir.Validate();

	if (dir.IsEmpty())
	{
		dir.InsertBack('.');
		ValidatePathName( dir );
	}

	if (dir.IsEmpty())
		dir.InsertBack('.');

	if (dir.Back() != '\\' && dir.Back() != '/')
		dir.InsertBack('\\');
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GetExeDir(PDXSTRING& dir)
{
	dir.Buffer().Resize( MAX_PATH + 1 );
	dir.Buffer().Front() = '\0';

	if (!::GetModuleFileName( NULL, dir.Begin(), MAX_PATH ))
		throw ("GetModuleFileName() failed!");

	dir.Validate();

	if (!ValidateDir( dir ))
		throw ("GetModuleFileName() failed!");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GetExeName(PDXSTRING& path)
{
	path.Buffer().Resize( MAX_PATH + 1 );
	path.Buffer().Front() = '\0';

	if (!::GetModuleFileName( NULL, path.Begin(), MAX_PATH ))
		throw ("GetModuleFileName() failed!");

	path.Validate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL PathExist(const PDXSTRING& path)
{
	return path.Length() && (::GetFileAttributes( path.String() ) != INVALID_FILE_ATTRIBUTES);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FileExist(const PDXSTRING& path)
{
	if (path.IsEmpty())
		return FALSE;

	const DWORD result = ::GetFileAttributes( path.String() );

	if (result == INVALID_FILE_ATTRIBUTES || (result & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DirExist(const PDXSTRING& path)
{
	if (path.IsEmpty())
		return FALSE;

	if (::GetFileAttributes( path.String() ) != INVALID_FILE_ATTRIBUTES)
		return TRUE;

	CHAR drive[_MAX_DRIVE+1];
	CHAR folders[_MAX_DIR+1];

	::_splitpath( path.String(), drive, folders, NULL, NULL );

	CHAR full[_MAX_PATH+1];
	strcat( full, drive   );
	strcat( full, folders );

	const TSIZE length = strlen( full );

	if (!length || (full[length-1] != '\\' && full[length-1] != '/'))
		return FALSE;

	const DWORD result = ::GetFileAttributes( full );

	if (result == INVALID_FILE_ATTRIBUTES || !(result & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL ValidatePathName(PDXSTRING& path)
{
	if (path.IsEmpty())
		return FALSE;

	CHAR full[_MAX_PATH+1];

	if (!::_fullpath( full, path.String(), MAX_PATH ))
	{
		path.Clear();
		return FALSE;
	}

	path = full;
	
	return path.Length() ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL ValidatePath(PDXSTRING& path)
{
	const DWORD result = ::GetFileAttributes( path.String() );

	if (result == INVALID_FILE_ATTRIBUTES)
	{
		path.Clear();
		return FALSE;
	}

	if ((result & FILE_ATTRIBUTE_DIRECTORY) && path.Back() != '\\' && path.Back() != '/')
		path.InsertBack('\\');

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL ValidateFile(PDXSTRING& file)
{
	return ValidatePath( file ) && FileExist( file );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL ValidateDir(PDXSTRING& dir)
{
	if (!ValidatePath( dir ))
		return FALSE;

	PDXSTRING::CONSTITERATOR pos = dir.AtLastOf( "\\" );

	if (pos == dir.End())
	{
		pos = dir.AtLastOf( "/" );
		PDX_ASSERT( pos != dir.End() );
	}

	dir.Resize( (pos - dir.Begin()) + 1 );

	const DWORD result = ::GetFileAttributes( dir.String() );

	if (result == INVALID_FILE_ATTRIBUTES || !(result & FILE_ATTRIBUTE_DIRECTORY))
	{
		dir.Clear();
		return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID FromGUID(const GUID& guid,PDXSTRING& string)
{
	string.Buffer().Resize(36+1);

	CHAR* const buffer = string.Buffer().Begin();

	sprintf( buffer +  0, "%08X", guid.Data1    );
	sprintf( buffer +  9, "%04X", guid.Data2    );
	sprintf( buffer + 14, "%04X", guid.Data3    );
	sprintf( buffer + 19, "%02X", guid.Data4[0] );
	sprintf( buffer + 21, "%02X", guid.Data4[1] );

	for (UINT i=0; i < 8-2; ++i)
		sprintf( buffer + 24 + (i*2), "%02X", guid.Data4[i+2] );

	buffer[8] = buffer[13] = buffer[18] = buffer[23] = '-';

	string.Validate();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID ToGUID(const CHAR* const string,GUID& guid)
{
	if (string && strlen(string) == 36 && string[8] == '-' && string[13] == '-' && string[18] == '-' && string[23] == '-')
	{
		CHAR buffer[2+8+1];

		buffer[0] = '0';
		buffer[1] = 'X';

		memcpy( buffer + 2, string +  0, 8 ); buffer[2+8] = '\0'; guid.Data1    = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string +  9, 4 ); buffer[2+4] = '\0'; guid.Data2    = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string + 14, 4 ); buffer[2+4] = '\0'; guid.Data3    = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string + 19, 2 ); buffer[2+2] = '\0'; guid.Data4[0] = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string + 21, 2 ); buffer[2+2] = '\0'; guid.Data4[1] = strtoul( buffer, NULL, 0 );

		for (UINT i=0; i < 8-2; ++i)
		{
			memcpy( buffer + 2, string + 24 + (i*2), 2 );
			buffer[2+2] = '\0';
			guid.Data4[i+2] = strtoul( buffer, NULL, 0 );
		}
	}
	else
	{
		PDXMemZero( guid );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL BrowseFolder(PDXSTRING& folder,HWND hWnd,const UINT TitleID)
{
	PDXSTRING title;

	CHAR name[MAX_PATH+1];
	name[0] = '\0';

	BROWSEINFO bi;
	PDXMemZero( bi );

	bi.hwndOwner	  = hWnd;
	bi.pszDisplayName = name;
	bi.lpszTitle	  = IdToString( TitleID, title );
	bi.ulFlags		  = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST idl = ::SHBrowseForFolder( &bi );

	BOOL yep = FALSE;

	if (idl)
	{
		if (::SHGetPathFromIDList( idl, name ) && name[0] != '\0')
		{
			folder = name;

			if (folder.Back() != '\\' && folder.Back() != '/')
				folder.InsertBack('\\');

			yep = TRUE;
		}

		LPMALLOC pMalloc;

		if (SUCCEEDED(::SHGetMalloc( &pMalloc ))) 
		{
			pMalloc->Free( idl );
			pMalloc->Release();
		}
	}

	return yep;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL BrowseOpenFile
( 
	PDXSTRING& filename, 
	HWND hWnd,
	const UINT titleID,
	const CHAR* const filter,
	const CHAR* const path
)
{
	PDXSTRING title;
	IdToString( titleID, title );
	return BrowseOpenFile( filename, hWnd, title.String(), filter, path );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL BrowseOpenFile
(
    PDXSTRING& filename,
	HWND hWnd,
	const CHAR* const title,
	const CHAR* const filter,
	const CHAR* const path
)
{
	filename.Buffer().Resize( MAX_PATH+1 );
	filename.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = hWnd ? hWnd : application.GetHWnd();
	ofn.lpstrFile   = filename.Begin();
	ofn.lpstrTitle  = title;
	ofn.nMaxFile    = MAX_PATH;
	ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (filter)
	{
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
	}

	PDXSTRING DefPath;

	if (path)
	{
		ofn.lpstrInitialDir	= path;
	}
	else
	{
		GetCurrentDir( DefPath );
		ofn.lpstrInitialDir	= DefPath.String();
	}

	if (!::GetOpenFileName( &ofn ))
	{
		filename.Clear();
		return FALSE;
	}

	filename.Validate();

	return filename.Length() ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL BrowseSaveFile
( 
    PDXSTRING& filename, 
    HWND hWnd,
    const UINT titleID,
    const CHAR* const filter,
    const CHAR* const path,
    const CHAR* const DefExt
)
{
	PDXSTRING title;
	IdToString( titleID, title );
	return BrowseSaveFile( filename, hWnd, title.String(), filter, path, DefExt );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL BrowseSaveFile
( 
    PDXSTRING& filename, 
	HWND hWnd,
	const CHAR* const title,
	const CHAR* const filter,
	const CHAR* const path,
	const CHAR* const DefExt
)
{
	filename.Buffer().Resize( MAX_PATH+1 );
	filename.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = hWnd ? hWnd : application.GetHWnd();
	ofn.lpstrFile   = filename.Begin();
	ofn.lpstrTitle  = title;
	ofn.lpstrDefExt = DefExt;
	ofn.nMaxFile    = MAX_PATH;
	ofn.Flags       = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if (filter)
	{
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
	}

	PDXSTRING DefPath;

	if (path)
	{
		ofn.lpstrInitialDir	= path;
	}
	else
	{
		GetCurrentDir( DefPath );
		ofn.lpstrInitialDir	= DefPath.String();
	}

	if (!::GetOpenFileName( &ofn ))
	{
		filename.Clear();
		return FALSE;
	}

	filename.Validate();

	return filename.Length() ? TRUE : FALSE;
}

}
