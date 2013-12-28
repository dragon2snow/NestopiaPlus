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

#include <Windows.h>
#include "resource/resource.h"
#include "NstApplication.h"
#include "NstMovieManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::Play() 
{
	if (file.Length())
	{
		if (PDX_SUCCEEDED(nes->LoadMovie(file)))		
			nes->StartMovie();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::Stop() 
{
	if (nes->IsMoviePlaying() || nes->IsMovieRecording())
		nes->StopMovie();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::Record() 
{
	if (file.Length())
	{
		if (PDX_SUCCEEDED(nes->SaveMovie(file)))
			nes->StartMovie();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::Rewind() 
{
	if (file.Length())
		nes->RewindMovie();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::UpdateDialog(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_MOVIE_FILE, file );	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::UpdateSettings(HWND hDlg)
{
	file.Clear();
	file.Buffer().Resize( NST_MAX_PATH );

	GetDlgItemText( hDlg, IDC_MOVIE_FILE, file.Begin(), NST_MAX_PATH );

	file.Validate();

	if (file.Size())
	{
		if (file.GetFileExtension().IsEmpty())
			file += ".nsv";
	}
	else
	{
		nes->CloseMovie();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::OnBrowse(HWND hDlg)
{
	PDXSTRING file;
	file.Buffer().Resize( NST_MAX_PATH );
	file.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.hInstance       = application.GetHInstance();
	ofn.lpstrFilter     = "NES Movie File (*.nsv)\0*.nsv\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir	= application.GetStatePath();
	ofn.lpstrFile       = file.Begin();
	ofn.lpstrTitle      = "Select NES Movie File";
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&ofn))
	{
		file.Validate();

		if (file.Length() && file.GetFileExtension().IsEmpty())
			file += ".nsv";

		SetDlgItemText( hDlg, IDC_MOVIE_FILE, file );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::OnClear(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_MOVIE_FILE, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL MOVIEMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			UpdateDialog( hDlg );
			return TRUE;

		case WM_COMMAND:

         	switch (LOWORD(wParam))
			{
    			case IDC_MOVIE_CLEAR:  OnClear( hDlg ); return TRUE;
    			case IDC_MOVIE_BROWSE: OnBrowse( hDlg ); return TRUE;
				case IDC_MOVIE_OK:     UpdateSettings( hDlg );
				case IDC_MOVIE_CANCEL: EndDialog( hDlg, 0 ); return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}