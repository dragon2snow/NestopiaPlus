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

#include <Windows.h>
#include "NstMovieManager.h"
#include "NstFileManager.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::Load(PDXSTRING& name)
{
	if (name.Size())
	{
		if (name.GetFileExtension().IsEmpty())
			name += ".nsv";

		file = name;
		nes.LoadMovie( file );
	}
	else
	{
		file.Clear();
		nes.CloseMovie();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
														 
VOID MOVIEMANAGER::Play()    { if (nes.CanPlayMovie())    nes.PlayMovie();    } 
VOID MOVIEMANAGER::Stop()    { if (nes.CanStopMovie())    nes.StopMovie();    }
VOID MOVIEMANAGER::Record()  { if (nes.CanRecordMovie())  nes.RecordMovie();  }
VOID MOVIEMANAGER::Rewind()  { if (nes.CanRewindMovie())  nes.RewindMovie();  }
VOID MOVIEMANAGER::Forward() { if (nes.CanForwardMovie()) nes.ForwardMovie(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::UpdateDialog(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_MOVIE_FILE, file.String() );	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::UpdateSettings(HWND hDlg)
{
	PDXSTRING name;
	MANAGER::GetDlgItemText( hDlg, IDC_MOVIE_FILE, name );
	Load( name );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::OnBrowse(HWND hDlg)
{
	PDXSTRING filename;

	const BOOL succeeded = UTILITIES::BrowseSaveFile
	(
	    filename,
		hDlg,
		IDS_FILE_SELECT_MOVIE,
    	"Nestopia Movie File (*.nsv)\0"
		"*.nsv\0"
		"All Files (*.*)\0"
		"*.*\0",
		application.GetFileManager().GetNstPath().String(),
		"nsv"
	);

	if (succeeded)
		::SetDlgItemText( hDlg, IDC_MOVIE_FILE, filename.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MOVIEMANAGER::OnClear(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_MOVIE_FILE, "" );
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
				case IDC_MOVIE_CANCEL: ::EndDialog( hDlg, 0 ); return TRUE;
			}		
			return FALSE;

     	case WM_CLOSE:

     		::EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
