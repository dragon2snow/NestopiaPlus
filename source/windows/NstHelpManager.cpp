///////////////////////////////////////////////////////////////////////////////////////
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
#include <ShellAPI.h>
#include "NstHelpManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

HELPMANAGER::HELPMANAGER()
:
about   (IDD_ABOUT),
licence (IDD_LICENCE)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const CHAR* HELPMANAGER::ImportResource(const CHAR* name,const UINT id)
{
	HGLOBAL hGlobal;
	HMODULE hModule = ::GetModuleHandle( NULL );		
	HRSRC hRsrc = ::FindResource( hModule, MAKEINTRESOURCE(id), name );

	if (hRsrc && (hGlobal = ::LoadResource( hModule, hRsrc )) && ::SizeofResource( hModule, hRsrc ))
		return (const CHAR*) ::LockResource( hGlobal );

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL HELPMANAGER::ABOUT::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
       	case WM_SETCURSOR:
		{
			const BOOL touching =
			(
			    HWND(wParam) == ::GetDlgItem( hDlg, IDC_ABOUT_GNU  ) || 
				HWND(wParam) == ::GetDlgItem( hDlg, IDC_ABOUT_URL1 ) ||
				HWND(wParam) == ::GetDlgItem( hDlg, IDC_ABOUT_URL2 )
			);

			if (touching)
			{
				::SetCursor( ::LoadCursor(NULL,IDC_UPARROW) );
				::SetWindowLong( hDlg, DWL_MSGRESULT, MAKELONG(TRUE,0) );
			}
			else
			{
				::SetCursor( ::LoadCursor(NULL,IDC_ARROW) );
				::SetWindowLong( hDlg, DWL_MSGRESULT, MAKELONG(TRUE,0) );
			}
			return TRUE;
		}

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
		       	case IDC_ABOUT_GNU:
				case IDC_ABOUT_URL1:
				case IDC_ABOUT_URL2:
				{
					PDXSTRING url;

					if (MANAGER::GetDlgItemText( hDlg, LOWORD(wParam), url ))
						::ShellExecute( hDlg, "open", url.Begin(), NULL, NULL, SW_SHOWNORMAL );

					return TRUE;
				}
			}		
			return FALSE;

     	case WM_CLOSE:

     		::EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL HELPMANAGER::LICENCE::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:
		{
			const CHAR* string;

			if (string = ImportResource( "copying", IDR_COPYING1 ))
				::SetDlgItemText( hDlg, IDC_LICENCE_EDIT, string );

			return TRUE;
		}
				    
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_LICENCE_OK:
					
					::EndDialog( hDlg, 0 ); 
					return TRUE;
			}		
			return FALSE;
	}

	return FALSE;
}
