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
#include <WindowsX.h>
#include <ShellAPI.h>
#include "NstApplication.h"
#include "NstUtilities.h"
#include "NstFileManager.h"
#include "NstNetplayManager.h"

#define NETPLAYLIST_FILENAME "netplaylist.dat"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NETPLAYMANAGER::NETPLAYMANAGER()
: MANAGER(IDD_NETPLAY) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	UseDatabaseNames = TRUE;
	PlayFullscreen = FALSE;

	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		if (file["netplay use database names"] == "no")
			UseDatabaseNames = FALSE;

		if (file["netplay in fullscreen"] == "yes")
			PlayFullscreen = TRUE;
	}

	PDXFILE file;

	PDXSTRING name;
	UTILITIES::GetExeDir( name );
	name << NETPLAYLIST_FILENAME;

	if (PDX_FAILED(file.Open( name, PDXFILE::INPUT )))
		return;
	
	PDXSTRING string;

	for (;;)
	{
		CHAR c = '\0';

		if (file.Readable(sizeof(CHAR)))
			c = file.Read<CHAR>();

		if (c == '\r' || c == '\n' || c == '\0')
		{
			if (string.Length())
			{
				if (UTILITIES::ValidatePath( string ))
					GameList.InsertBack( string );

				string.Clear();
			}

			if (c == '\0')
				break;
		}
		else
		{
			string.InsertBack( c );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		file[ "netplay use database names" ] = (UseDatabaseNames ? "yes" : "no");
		file[ "netplay in fullscreen"      ] = (PlayFullscreen ? "yes" : "no");
	}

	PDXSTRING name;
	UTILITIES::GetExeDir( name );
	name << NETPLAYLIST_FILENAME;

	if (UTILITIES::FileExist( name ))
		::DeleteFile( name.String() );

	if (GameList.Size())
	{
		PDXFILE file( name, PDXFILE::OUTPUT );

		if (file.IsOpen())
		{
			for (UINT i=0; i < GameList.Size(); ++i)
			{
				file.Write( GameList[i].Begin(), GameList[i].End() );
				file.Write( '\r' );
				file.Write( '\n' );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::Start()
{
	if (StartDialog() == START_KAILLERA)
		kaillera.Launch( GameList, UseDatabaseNames, PlayFullscreen );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::UpdateDialog(HWND hDlg)
{
	::EnableWindow( ::GetDlgItem( hDlg, IDC_NETPLAY_PLAY_FULLSCREEN ), application.IsWindowed() ? TRUE : FALSE );

	::CheckDlgButton( hDlg, IDC_NETPLAY_DATABASENAMES, (UseDatabaseNames ? BST_CHECKED : BST_UNCHECKED) );
	::CheckDlgButton( hDlg, IDC_NETPLAY_PLAY_FULLSCREEN, (PlayFullscreen ? BST_CHECKED : BST_UNCHECKED) );
	
	ListBox_Enable( ::GetDlgItem( hDlg, IDC_NETPLAY_START_KAILLERA ), GameList.Size() ? TRUE : FALSE );

	HWND hList = ::GetDlgItem( hDlg, IDC_NETPLAY_GAMELIST );	

	for (TSIZE i=0; i < GameList.Size(); ++i)
		ListBox_AddString( hList, GameList[i].String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::OnAdd(HWND hDlg)
{
	PDXSTRING filename;

	const BOOL success = UTILITIES::BrowseOpenFile
	(
	    filename,
		hDlg,
		UTILITIES::IdToString(IDS_FILE_ADD).String(),
		(
			"All supported Files\0"
			"*.nes;*.unf;*.fds;*.nsp;*.zip\0"
			"iNes ROM Images (*.nes)\0"
			"*.nes\0"
			"UNIF ROM Images (*.unf)\0"
			"*.unf\0"
			"Famicom Disk System Images (*.fds)\0"
			"*.fds\0"
			"Nestopia Script Files (*.nsp)\0"
			"*.nsp\0"
			"Zip Files (*.zip)\0"
			"*.zip\0"
			"All Files (*.*)\0"
			"*.*\0"									   
		),
		application.GetFileManager().GetRomPath().String()
	);

	if (success && filename.Length())
	{
		ListBox_AddString( ::GetDlgItem( hDlg, IDC_NETPLAY_GAMELIST ), filename.String() );
		ListBox_Enable( ::GetDlgItem( hDlg, IDC_NETPLAY_START_KAILLERA ), TRUE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::OnRemove(HWND hDlg)
{
	HWND hList = ::GetDlgItem( hDlg, IDC_NETPLAY_GAMELIST );	
	const INT selected = ListBox_GetCurSel( hList );

	if (selected != -1)
	{
		ListBox_DeleteString( hList, selected );
		
		if (!ListBox_GetCount( hList ))
     		ListBox_Enable( ::GetDlgItem( hDlg, IDC_NETPLAY_START_KAILLERA ), FALSE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::OnClearAll(HWND hDlg)
{
	ListBox_ResetContent( ::GetDlgItem( hDlg, IDC_NETPLAY_GAMELIST ) );	
	ListBox_Enable( ::GetDlgItem( hDlg, IDC_NETPLAY_START_KAILLERA ), FALSE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::UpdateSettings(HWND hDlg)
{
	UseDatabaseNames = (::IsDlgButtonChecked( hDlg, IDC_NETPLAY_DATABASENAMES ) == BST_CHECKED);
	PlayFullscreen = (::IsDlgButtonChecked( hDlg, IDC_NETPLAY_PLAY_FULLSCREEN ) == BST_CHECKED);

	HWND hList = ::GetDlgItem( hDlg, IDC_NETPLAY_GAMELIST );	
	const INT count = ListBox_GetCount( hList );

	if (count > 0)
	{
		GameList.Resize( count );

		for (UINT i=0; i < count; ++i)
		{
			GameList[i].Resize( ListBox_GetTextLen( hList, i ) + 1 );
			GameList[i].Front() = '\0';
			ListBox_GetText( hList, i, GameList[i].Buffer().Begin() );
			GameList[i].Validate();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::OnDefault(HWND hDlg)
{
	::CheckDlgButton( hDlg, IDC_NETPLAY_DATABASENAMES, BST_CHECKED );
	::CheckDlgButton( hDlg, IDC_NETPLAY_PLAY_FULLSCREEN, BST_UNCHECKED );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID NETPLAYMANAGER::OnDropFiles(HWND hDlg,WPARAM wParam)
{
	HWND hList = ::GetDlgItem( hDlg, IDC_NETPLAY_GAMELIST );

	PDXSTRING file;
	const UINT NumGames = ::DragQueryFile( HDROP(wParam), 0xFFFFFFFFU, NULL, 0 );

	for (UINT i=0; i < NumGames; ++i)
	{
		file.Resize( MAX_PATH+1 );
		file.Front() = '\0';

		if (::DragQueryFile( HDROP(wParam), 0, file.Begin(), MAX_PATH ))
		{
			file.Validate();

			if (file.Length())
			{
				ListBox_AddString( hList, file.String() );
				ListBox_Enable( ::GetDlgItem( hDlg, IDC_NETPLAY_START_KAILLERA ), TRUE );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL NETPLAYMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg)
	{
     	case WM_INITDIALOG:

     		UpdateDialog( hDlg );
     		return TRUE;

     	case WM_COMMAND:

       		switch (LOWORD(wParam))
     		{
          		case IDC_NETPLAY_ADD:    
					
					OnAdd( hDlg ); 
					return TRUE;

           		case IDC_NETPLAY_REMOVE: 
					
					OnRemove( hDlg ); 
					return TRUE;

				case IDC_NETPLAY_CLEAR_ALL:

					OnClearAll( hDlg );
					return TRUE;

				case IDC_NETPLAY_DEFAULT:

					OnDefault( hDlg );
					return TRUE;

          		case IDC_NETPLAY_CANCEL: 
					
					UpdateSettings( hDlg );
					::EndDialog( hDlg, 0 ); 
					return TRUE;

          		case IDC_NETPLAY_START_KAILLERA: 
					
					UpdateSettings( hDlg );
					::EndDialog( hDlg, START_KAILLERA ); 
					return TRUE;
     		}		
     		return FALSE;

		case WM_DROPFILES:

			OnDropFiles( hDlg, wParam );
			return 0;

     	case WM_CLOSE:

			UpdateSettings( hDlg );
     		::EndDialog( hDlg, 0 );
     		return TRUE;
	}

	return FALSE;
}
