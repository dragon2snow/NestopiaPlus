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

MANAGER::MANAGER(const INT did)
: 
nes       (NULL), 
hWnd      (NULL),
hInstance (NULL),
DialogID  (did)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MANAGER::Init(HWND w,HINSTANCE i,NES::MACHINE* const n,CONFIGFILE* const ConfigFile)
{
	hWnd = w;
	hInstance = i;
	nes = n;
	return Create( ConfigFile );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MANAGER::Close(CONFIGFILE* const ConfigFile)
{
	hWnd = NULL;
	hInstance = NULL;
	PDXRESULT result = Destroy( ConfigFile );
	nes = NULL;
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MANAGER::StartDialog()
{
	PDX_ASSERT( hWnd && DialogID != INT_MAX );

	DialogBoxParam
	(
	    application.GetInstance(),
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
