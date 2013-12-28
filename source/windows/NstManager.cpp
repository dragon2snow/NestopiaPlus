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

#include <Windows.h>
#include "../paradox/PdxFile.h"
#include "NstManager.h"
#include "NstTimerManager.h"
#include "NstSoundManager.h"
#include "NstGraphicManager.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MANAGER::MANAGER(const INT id)
: 
hWnd     (application.GetHWnd()),
nes      (application.GetNes()), 
DialogID (id)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MANAGER::StartDialog()
{
	PDX_ASSERT( hWnd && DialogID != INT_MAX );

	DialogBoxParam
	(
		::GetModuleHandle(NULL),
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
	static PDXARRAY<PDXPAIR<HWND,MANAGER* const> > stack;

	BOOL GotIt = FALSE;

	if (uMsg == WM_INITDIALOG)
	{
		GotIt = TRUE;

		if (stack.IsEmpty() || stack.Back().First() != hDlg)
		{
			if (stack.IsEmpty())
			{
				application.GetSoundManager().Clear();
				application.GetTimerManager().Update();
				application.GetGraphicManager().BeginDialogMode();
			}

			stack.InsertBack( PDX::MakePair(hDlg,PDX_CAST(MANAGER* const,lParam)) );
		}
	}

	if 
	(
     	stack.Size() && 
		stack.Back().First() == hDlg && 
		stack.Back().Second() && 
		stack.Back().Second()->DialogProc( hDlg, uMsg, wParam, lParam )
	)
		GotIt = TRUE;

	if (uMsg == WM_DESTROY)
	{
		GotIt = TRUE;

		if (stack.Size() && stack.Back().First() == hDlg)
		{
			stack.EraseBack();

			if (stack.IsEmpty())
				application.GetGraphicManager().EndDialogMode();
		}
	}

	return GotIt;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

TSIZE MANAGER::GetDlgItemText(HWND hDlg,const INT id,PDXSTRING& string,const INT MaxLength)
{
	PDX_ASSERT( hDlg && id );

	string.Buffer().Resize( MaxLength + 1 );
	string.Buffer().Front() = '\0';
	string.Buffer().Resize( ::GetDlgItemText( hDlg, id, string.Begin(), MaxLength ) + 1 );

	return string.Length();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL FILEEXISTDIALOG::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
     		case IDC_FILEEXIST_OVERWRITE:
			case IDC_FILEEXIST_APPEND:
			case IDC_FILEEXIST_CANCEL:

				choice = LOWORD(wParam);
				::EndDialog( hDlg, 0 );
				return TRUE;
		}
	}

	return FALSE;
}

