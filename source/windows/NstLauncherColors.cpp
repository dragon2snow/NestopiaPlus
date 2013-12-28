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

#include <Windows.h>
#include "NstLauncherColors.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_DEF_FG RGB(0x00,0x00,0x00)
#define NST_DEF_BG RGB(0xFF,0xFF,0xFF)

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

LAUNCHERCOLORS::LAUNCHERCOLORS()
: 
MANAGER (IDD_LAUNCHER_COLORS),
FgColor (NST_DEF_FG),
BgColor (NST_DEF_BG)
{
	::SetRect( &rcFg, 20, 33, 74, 53  );
	::SetRect( &rcBg, 20, 94, 74, 114 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLORS::Create(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& cfg = *ConfigFile;

		const PDXSTRING& sFg = cfg["launcher color foreground"];
		const PDXSTRING& sBg = cfg["launcher color background"];

		if (sFg.Length())
			FgColor = strtoul( sFg.String(), NULL, 16 );

		if (sBg.Length())
			BgColor = strtoul( sBg.String(), NULL, 16 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLORS::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		PDXSTRING string;

		string.Set( FgColor, PDXSTRING::HEX );
		(*ConfigFile)["launcher color foreground"] = string;

		string.Set( BgColor, PDXSTRING::HEX );
		(*ConfigFile)["launcher color background"] = string;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL LAUNCHERCOLORS::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
       	case WM_PAINT:

			OnPaint( hDlg, rcFg, FgColor );
			OnPaint( hDlg, rcBg, BgColor );
			return FALSE;

     	case WM_COMMAND:

     		switch (LOWORD(wParam))
     		{
       			case IDC_LAUNCHER_COLORS_BG_CHANGE:

					OnChange( hDlg, BgColor );
					OnPaint( hDlg, rcBg, BgColor );
					return TRUE;

				case IDC_LAUNCHER_COLORS_FG_CHANGE:

					OnChange( hDlg, FgColor );
					OnPaint( hDlg, rcFg, FgColor );
					return TRUE;

          		case IDC_LAUNCHER_COLORS_DEFAULT: 

					OnPaint( hDlg, rcFg, (FgColor=NST_DEF_FG) );
					OnPaint( hDlg, rcBg, (BgColor=NST_DEF_BG) );
					return TRUE;

				case IDC_LAUNCHER_COLORS_OK: 
					
					::EndDialog( hDlg, 0 ); 
					return TRUE;
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

VOID LAUNCHERCOLORS::OnChange(HWND hDlg,COLORREF& current)
{
	CHOOSECOLOR cc;
	PDXMemZero( cc );

	cc.lStructSize  = sizeof(cc);
	cc.hwndOwner    = hDlg;
	cc.lpCustColors = CustomColors;
	cc.rgbResult    = current;
	cc.Flags        = CC_FULLOPEN|CC_RGBINIT;

	if (::ChooseColor( &cc )) 
		current = cc.rgbResult;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID LAUNCHERCOLORS::OnPaint(HWND hDlg,const RECT& rect,const DWORD color)
{
	HDC hDC = ::GetDC( hDlg );
  
	HPEN hPen = ::CreatePen( PS_SOLID, 1, RGB(0x00,0x00,0x00) );
	HPEN hPenOld = HPEN(::SelectObject( hDC, hPen ));

	HBRUSH hBrush = ::CreateSolidBrush( color );
	HBRUSH hBrushOld = HBRUSH(::SelectObject( hDC, hBrush ));

	::Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );

	::SelectObject( hDC, hBrushOld );
	::DeleteObject( hBrush );
	
	::SelectObject( hDC, hPenOld );
	::DeleteObject( hPen );

	::ReleaseDC( hWnd, hDC );
}
