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

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <cstdio>
#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include "NstLauncherPaths.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHERPATHS::LAUNCHERPATHS()
: 
MANAGER (IDD_LAUNCHER_PATHS),
flags   (PATH_INC_ALL)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::Create(CONFIGFILE* const ConfigFile)
{
	PDX_ASSERT( folders.IsEmpty() );

	if (ConfigFile)
	{
		CONFIGFILE& cfg = *ConfigFile;

		const UINT NumFolders = cfg["launcher search paths"].ToUlong();

		if (NumFolders)
		{
			PDXSTRING index("launcher search path ");

			for (UINT i=0; i < NumFolders; ++i)
			{
				folders.Grow();
				FOLDER& folder = folders.Back();

				index.Resize(21);
				index << (i+1);

				folder.folder = cfg[index];

				if (!UTILITIES::ValidateDir( folder.folder ))
				{
					folders.EraseBack();
					continue;
				}

				index << " sub-paths";
				folder.IncSubFolders = (cfg[index] == "yes");
			}
		}

		flags = 0;
		flags |= ( cfg[ "launcher search files nes"          ] == "no"  ? 0 : PATH_INC_NES       );
		flags |= ( cfg[ "launcher search files unf"          ] == "no"  ? 0 : PATH_INC_UNF       );
		flags |= ( cfg[ "launcher search files fds"          ] == "no"  ? 0 : PATH_INC_FDS       );
		flags |= ( cfg[ "launcher search files nsf"          ] == "no"  ? 0 : PATH_INC_NSF       );
		flags |= ( cfg[ "launcher search files nsp"          ] == "no"  ? 0 : PATH_INC_NSP       );
		flags |= ( cfg[ "launcher search files zip"          ] == "no"  ? 0 : PATH_INC_ZIP       );
		flags |= ( cfg[ "launcher search any file extension" ] == "yes" ? PATH_SEARCH_ALL    : 0 );
		flags |= ( cfg[ "launcher search no dublicate files" ] == "yes" ? PATH_NO_DUBLICATES : 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& cfg = *ConfigFile;

		cfg["launcher search paths"] = folders.Size();

		if (folders.Size())
		{
			PDXSTRING index("launcher search path ");

			for (UINT i=0; i < folders.Size(); ++i)
			{
				index.Resize(21);
				index << (i+1);

				PDX_ASSERT( folders[i].folder.Length() );
				cfg[index] = folders[i].folder.Quoted();

				index << " sub-paths";
				cfg[index] = (folders[i].IncSubFolders ? "yes" : "no");
			}
		}

		cfg[ "launcher search files nes"          ] = ( ( flags & PATH_INC_NES       ) ? "yes" : "no" );
		cfg[ "launcher search files unf"          ] = ( ( flags & PATH_INC_UNF       ) ? "yes" : "no" );
		cfg[ "launcher search files fds"          ] = ( ( flags & PATH_INC_FDS       ) ? "yes" : "no" );
		cfg[ "launcher search files nsf"          ] = ( ( flags & PATH_INC_NSF       ) ? "yes" : "no" );
		cfg[ "launcher search files nsp"          ] = ( ( flags & PATH_INC_NSP       ) ? "yes" : "no" );
		cfg[ "launcher search files zip"          ] = ( ( flags & PATH_INC_ZIP       ) ? "yes" : "no" );
		cfg[ "launcher search any file extension" ] = ( ( flags & PATH_SEARCH_ALL    ) ? "yes" : "no" );
		cfg[ "launcher search no dublicate files" ] = ( ( flags & PATH_NO_DUBLICATES ) ? "yes" : "no" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::Update(HWND hDlg)
{
	HWND hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_LIST );
	ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES );

	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_NES,         ( flags & PATH_INC_NES       ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_UNF,         ( flags & PATH_INC_UNF       ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_FDS,         ( flags & PATH_INC_FDS       ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_NSF,         ( flags & PATH_INC_NSF       ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_NSP,         ( flags & PATH_INC_NSP       ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_ZIP,         ( flags & PATH_INC_ZIP       ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_ALLFILES,    ( flags & PATH_SEARCH_ALL    ) ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hDlg, IDC_LAUNCHER_PATHS_UNIQUEFILES, ( flags & PATH_NO_DUBLICATES ) ? BST_CHECKED : BST_UNCHECKED );

	{
		LVITEM item;
		PDXMemZero( item );

		item.mask = LVIF_TEXT;
		item.iItem = INT_MAX;

		for (TSIZE i=0; i < folders.Size(); ++i)
		{
			item.pszText = folders[i].folder.Begin();
			item.cchTextMax = folders[i].folder.Length();

			const INT index = ListView_InsertItem( hList, &item );

			if (index == -1)
				throw ("ListView_InsertItem() failed!");

			ListView_SetCheckState( hList, index, folders[i].IncSubFolders );
		}
	}

	UpdateButtons( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::Save(HWND hDlg)
{
	flags = 0;
	
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_NES         ) == BST_CHECKED) flags |= PATH_INC_NES;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_UNF         ) == BST_CHECKED) flags |= PATH_INC_UNF;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_FDS         ) == BST_CHECKED) flags |= PATH_INC_FDS;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_NSF         ) == BST_CHECKED) flags |= PATH_INC_NSF;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_NSP         ) == BST_CHECKED) flags |= PATH_INC_NSP;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_ZIP         ) == BST_CHECKED) flags |= PATH_INC_ZIP;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_ALLFILES    ) == BST_CHECKED) flags |= PATH_SEARCH_ALL;
	if (::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_UNIQUEFILES ) == BST_CHECKED) flags |= PATH_NO_DUBLICATES;

	HWND hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_LIST );
	folders.Resize( ListView_GetItemCount( hList ) );

	for (TSIZE i=0; i < folders.Size(); ++i)
	{
		PDXSTRING& string = folders[i].folder;
		string.Resize( MAX_PATH+1 );

		ListView_GetItemText( hList, i, 0, string.Begin(), MAX_PATH );

		string.Validate();
		string.Defrag();

		folders[i].IncSubFolders = (ListView_GetCheckState( hList, i ) != 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::UpdateButtons(HWND hDlg)
{
	HWND hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_LIST );

	const BOOL HasItems = (ListView_GetItemCount( hList ) > 0);

	const BOOL CanOk =
	(
	    HasItems &&
		(
			(::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_NES ) == BST_CHECKED) ||
			(::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_UNF ) == BST_CHECKED) || 
			(::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_FDS ) == BST_CHECKED) || 
			(::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_NSF ) == BST_CHECKED) || 
			(::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_NSP ) == BST_CHECKED) || 
			(::IsDlgButtonChecked( hDlg, IDC_LAUNCHER_PATHS_ZIP ) == BST_CHECKED) 
		)
	);

	::EnableWindow( ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_REMOVE ), HasItems );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_CLEAR  ), HasItems );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_OK     ), CanOk    );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::OnAdd(HWND hDlg)
{
	PDXSTRING folder;
	
	if (UTILITIES::BrowseFolder( folder, hDlg, IDS_FILE_SELECTDIR ))
	{
		LVITEM item;
		PDXMemZero( item );

		item.mask       = LVIF_TEXT;
		item.pszText    = folder.Begin();
		item.cchTextMax = folder.Length();
		item.iItem      = INT_MAX;
		
		ListView_InsertItem( ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_LIST ), &item );
		UpdateButtons( hDlg );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::OnRemove(HWND hDlg)
{
	HWND hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_LIST );
	const INT item = ListView_GetSelectionMark( hList );

	if (item != -1)
	{
		ListView_DeleteItem( hList, item );
		UpdateButtons( hDlg );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERPATHS::OnClear(HWND hDlg)
{
	ListView_DeleteAllItems( ::GetDlgItem( hDlg, IDC_LAUNCHER_PATHS_LIST ) );
	UpdateButtons( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERPATHS::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg)
	{
       	case WM_INITDIALOG:

			Update( hDlg );
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_LAUNCHER_PATHS_NES:
				case IDC_LAUNCHER_PATHS_UNF:
				case IDC_LAUNCHER_PATHS_FDS:
				case IDC_LAUNCHER_PATHS_NSF:
				case IDC_LAUNCHER_PATHS_NSP:
				case IDC_LAUNCHER_PATHS_ZIP:

					UpdateButtons( hDlg );
					return FALSE;

     			case IDC_LAUNCHER_PATHS_ADD:    
					
					OnAdd( hDlg ); 
					return TRUE;

				case IDC_LAUNCHER_PATHS_REMOVE: 
					
					OnRemove( hDlg ); 
					return TRUE;

				case IDC_LAUNCHER_PATHS_CLEAR:  
					
					OnClear( hDlg ); 
					return TRUE;

				case IDC_LAUNCHER_PATHS_OK:     
					
					Save( hDlg );

				case IDC_LAUNCHER_PATHS_CANCEL: 
					
					::EndDialog( hDlg, 0 ); 
					return TRUE;		
			}
	}

	return FALSE;
}
