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
#include <WindowsX.h>
#include "../paradox/PdxString.h"
#include "NstVsDipSwitchManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL VSDIPSWITCHMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			InitDialog( hDlg );
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
       			case IDC_DIPSWITCHES_OK:     CloseDialog( hDlg );
				case IDC_DIPSWITCHES_CANCEL: ::EndDialog( hDlg, 0 ); return TRUE;
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

VOID VSDIPSWITCHMANAGER::InitDialog(HWND hDlg)
{
	PDX_COMPILE_ASSERT
	(
		IDC_DIPSWITCHES_2 - IDC_DIPSWITCHES_1 == 1 &&
		IDC_DIPSWITCHES_3 - IDC_DIPSWITCHES_1 == 2 &&
		IDC_DIPSWITCHES_4 - IDC_DIPSWITCHES_1 == 3 &&
		IDC_DIPSWITCHES_5 - IDC_DIPSWITCHES_1 == 4 &&
		IDC_DIPSWITCHES_6 - IDC_DIPSWITCHES_1 == 5 &&
		IDC_DIPSWITCHES_7 - IDC_DIPSWITCHES_1 == 6 &&
		IDC_DIPSWITCHES_8 - IDC_DIPSWITCHES_1 == 7
	);

	PDX_COMPILE_ASSERT
	(
		IDC_DIPSWITCHES_2_TEXT - IDC_DIPSWITCHES_1_TEXT == 1 &&
		IDC_DIPSWITCHES_3_TEXT - IDC_DIPSWITCHES_1_TEXT == 2 &&
		IDC_DIPSWITCHES_4_TEXT - IDC_DIPSWITCHES_1_TEXT == 3 &&
		IDC_DIPSWITCHES_5_TEXT - IDC_DIPSWITCHES_1_TEXT == 4 &&
		IDC_DIPSWITCHES_6_TEXT - IDC_DIPSWITCHES_1_TEXT == 5 &&
		IDC_DIPSWITCHES_7_TEXT - IDC_DIPSWITCHES_1_TEXT == 6 &&
		IDC_DIPSWITCHES_8_TEXT - IDC_DIPSWITCHES_1_TEXT == 7
	);

	NES::IO::DIPSWITCH::CONTEXT context;

	for (UINT i=0; i < 8; ++i)
	{
		HWND hCmd = ::GetDlgItem( hDlg, IDC_DIPSWITCHES_1 + i );
		HWND hTxt = ::GetDlgItem( hDlg, IDC_DIPSWITCHES_1_TEXT + i );

		if (nes.GetNumVsSystemDipSwitches() > i)
		{
			nes.GetVsSystemDipSwitch( i, context );

			::SetWindowText( hTxt, context.name.String() );

			for (UINT j=0; j < context.settings.Size(); ++j)
				ComboBox_AddString( hCmd, context.settings[j].String() );

			ComboBox_SetCurSel( hCmd, context.index );
		}
		else
		{
			::DestroyWindow( hTxt );
			::DestroyWindow( hCmd ); 
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID VSDIPSWITCHMANAGER::CloseDialog(HWND hDlg)
{
	NES::IO::DIPSWITCH::CONTEXT context;

	for (UINT i=0; i < nes.GetNumVsSystemDipSwitches(); ++i)
	{
		context.index = ComboBox_GetCurSel( ::GetDlgItem( hDlg, IDC_DIPSWITCHES_1 + i ) );
		nes.SetVsSystemDipSwitch( i, context );
	}
}
