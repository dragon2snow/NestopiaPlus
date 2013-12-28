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

#include "../paradox/PdxFile.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MANAGER::MANAGER(const INT did,const UINT chunk)
: 
nes       (NULL), 
hWnd      (NULL),
hInstance (NULL),
DialogID  (did),
FileChunk (chunk)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MANAGER::Init(HWND w,HINSTANCE i,NES::MACHINE* const n,PDXFILE* const file)
{
	hWnd = w;
	hInstance = i;
	nes = n;

	BOOL IsNull = TRUE;

	if (file && !file->IsEmpty() && FileChunk != NO_FILE)
	{
		if (FileChunk == file->Read<U8>())
		{
			IsNull = FALSE;
		}
		else
		{
			file->Seek( PDXFILE::CURRENT, -LONG(sizeof(U8)) );
		}
	}

	return Create( IsNull ? NULL : file );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MANAGER::Close(PDXFILE* const file)
{
	nes = NULL;
	hWnd = NULL;
	hInstance = NULL;

	BOOL IsNull = TRUE;

	if (file && FileChunk != NO_FILE)
	{
		IsNull = FALSE;
		file->Write( U8(FileChunk) );
	}
  
	return Destroy( IsNull ? NULL : file );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MANAGER::StartDialog()
{
	PDX_ASSERT( hWnd && DialogID != INT_MAX );

	DialogBoxParam
	(
	    application.GetHInstance(),
		MAKEINTRESOURCE(DialogID),
		hWnd,
		StaticDialogProc,
		PDX_CAST(LPARAM,this)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MANAGER::StaticDialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static MANAGER* manager = NULL;

	BOOL GotIt = FALSE;

	if (uMsg == WM_INITDIALOG)
	{
		PDX_ASSERT( !manager );
		manager = PDX_CAST(MANAGER*,lParam);
		application.BeginDialogMode();
		GotIt = TRUE;
	}

	if (manager && manager->DialogProc( hDlg, uMsg, wParam, lParam ))
		GotIt = TRUE;

	if (uMsg == WM_DESTROY)
	{
		manager = NULL;
		application.EndDialogMode();
		GotIt = TRUE;
	}

	return GotIt;
}
