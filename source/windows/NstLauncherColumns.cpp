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
#include "NstLauncherColumns.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const CHAR* LAUNCHERCOLUMNS::strings[NUM_TYPES] =
{
	"File",
	"System",
	"Mapper",
	"pRom",
	"cRom",
	"wRam",
	"Battery",
	"Trainer",
	"Mirroring",
	"Name",
	"Copyright",
	"Condition",
	"Folder"
};

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHERCOLUMNS::LAUNCHERCOLUMNS()
: MANAGER(IDD_LAUNCHER_COLUMNSELECT) 
{
	selected.Reserve( NUM_TYPES );
	available.Reserve( NUM_TYPES );
	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::Reset()
{
	selected.Resize( NUM_DEFAULT_SELECTED_TYPES );

	for (UINT i=0; i < NUM_DEFAULT_SELECTED_TYPES; ++i)
		selected[i] = TYPE(i);

	available.Resize( NUM_DEFAULT_AVAILABLE_TYPES );

	for (UINT i=0; i < NUM_DEFAULT_AVAILABLE_TYPES; ++i)
		available[i] = TYPE(NUM_DEFAULT_SELECTED_TYPES+i);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::Create(CONFIGFILE* const ConfigFile)
{
	if (!ConfigFile)
		return;

	CONFIGFILE& cfg = *ConfigFile;
	const UINT NumSelected = cfg["launcher columns"].ToUlong();

	if (!NumSelected)
		return;
	
	selected.Clear();
	available.Resize( NUM_TYPES );

	for (UINT i=0; i < NUM_TYPES; ++i)
		available[i] = TYPE(i);

	PDXSTRING index("launcher column ");

	for (UINT i=0; i < NumSelected; ++i)
	{
		index.Resize(16);
		index << (i+1);

		const PDXSTRING& type = cfg[index];

		if (type.IsEmpty())
			continue;
		
		for (UINT j=0; j < NUM_TYPES; ++j)
		{
			if (type == strings[j])
			{
				selected.InsertBack( TYPE(j) );

				for (UINT k=0; k < available.Size(); ++k)
				{
					if (available[k] == TYPE(j))
					{
						available.Erase( available.At(k) );
						break;
					}
				}
				break;
			}
		}
	}

	if (selected.IsEmpty())
		Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& cfg = *ConfigFile;

		cfg["launcher columns"] = selected.Size();

		PDXSTRING index("launcher column ");

		for (UINT i=0; i < selected.Size(); ++i)
		{
			index.Resize(16);
			index << (i+1);
			cfg[index] = strings[selected[i]];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::Update(HWND hDlg,const BOOL reset)
{
	HWND hList;
	
	hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_SELECTED );
	ListBox_ResetContent( hList );

	if (reset)
	{
		for (UINT i=0; i < NUM_DEFAULT_SELECTED_TYPES; ++i)
			ListBox_AddString( hList, strings[i] );
	}
	else
	{
		for (UINT i=0; i < selected.Size(); ++i)
			ListBox_AddString( hList, strings[selected[i]] );
	}

	hList = ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_AVAILABLE );
	ListBox_ResetContent( hList );

	if (reset)
	{
		for (UINT i=NUM_DEFAULT_SELECTED_TYPES; i < NUM_TYPES; ++i)
			ListBox_AddString( hList, strings[i] );
	}
	else
	{
		for (UINT i=0; i < available.Size(); ++i)
			ListBox_AddString( hList, strings[available[i]] );
	}

	UpdateButtons( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::Add(HWND hDlg,const UINT src,const UINT dst)
{
	HWND hListSrc = ::GetDlgItem( hDlg, src );

	const INT sSrc = ListBox_GetCurSel( hListSrc );
	PDX_ASSERT( sSrc != LB_ERR );

	PDXSTRING string;
	string.Resize( ListBox_GetTextLen( hListSrc, sSrc ) );
	ListBox_GetText( hListSrc, sSrc, string.Begin() );

	HWND hListDst = ::GetDlgItem( hDlg, dst );
	const INT sDst = ListBox_GetCurSel( hListDst );

	if (sDst == LB_ERR)
		ListBox_AddString( hListDst, string.String() );
	else
		ListBox_InsertString( hListDst, sDst+1, string.String() );

	ListBox_DeleteString( hListSrc, sSrc );

	if (sSrc < ListBox_GetCount( hListSrc ))
		ListBox_SetCurSel( hListSrc, sSrc );

	UpdateButtons( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::Save(HWND hDlg)
{
	PDXSTRING string;

	for (UINT i=0; i < 2; ++i)
	{
		HWND hList = ::GetDlgItem
		( 
	     	hDlg, 
			(i ? IDC_LAUNCHER_COLUMNSELECT_SELECTED : IDC_LAUNCHER_COLUMNSELECT_AVAILABLE)
		);

		TYPES& types = (i ? selected : available);
		types.Resize( ListBox_GetCount( hList ) );

		for (UINT j=0; j < types.Size(); ++j)
		{
			string.Resize( ListBox_GetTextLen( hList, j ) );
			ListBox_GetText( hList, j, string.Begin() );

			for (UINT k=0; k < NUM_TYPES; ++k)
			{
				if (string == strings[k])
				{
					types[j] = TYPE(k);
					break;
				}
				PDX_ASSERT( k < NUM_TYPES-1 );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLUMNS::UpdateButtons(HWND hDlg)
{
	HWND hListSel = ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_SELECTED  );
	HWND hListAva = ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_AVAILABLE );

	const BOOL bOk = (ListBox_GetCount( hListSel ) > 0);
	const BOOL bAddEnable = ((ListBox_GetCount( hListAva ) > 0) && (ListBox_GetCurSel( hListAva ) != LB_ERR));
	const BOOL bRemEnable = (bOk && (ListBox_GetCurSel( hListSel ) != LB_ERR));

	::EnableWindow( ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_ADD    ), bAddEnable );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_REMOVE ), bRemEnable );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_LAUNCHER_COLUMNSELECT_OK     ), bOk        );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERCOLUMNS::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg)
	{
       	case WM_INITDIALOG:

			Update( hDlg, FALSE );
			return TRUE;

		case WM_COMMAND:
		
			switch (LOWORD(wParam))
			{
         		case IDC_LAUNCHER_COLUMNSELECT_SELECTED:
				case IDC_LAUNCHER_COLUMNSELECT_AVAILABLE:

					UpdateButtons( hDlg );
					return FALSE;

       			case IDC_LAUNCHER_COLUMNSELECT_ADD:    
					
					Add( hDlg, IDC_LAUNCHER_COLUMNSELECT_AVAILABLE, IDC_LAUNCHER_COLUMNSELECT_SELECTED );
					return TRUE;

				case IDC_LAUNCHER_COLUMNSELECT_REMOVE:  
					
					Add( hDlg, IDC_LAUNCHER_COLUMNSELECT_SELECTED, IDC_LAUNCHER_COLUMNSELECT_AVAILABLE );
					return TRUE;

       			case IDC_LAUNCHER_COLUMNSELECT_DEFAULT: 
					
					Update( hDlg, TRUE );   
					return TRUE;

     			case IDC_LAUNCHER_COLUMNSELECT_OK:

					Save( hDlg );

				case IDC_LAUNCHER_COLUMNSELECT_CANCEL:  
					
					::EndDialog( hDlg, 0 ); 
					return TRUE;		
			}
	}

	return FALSE;
}
