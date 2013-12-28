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
#include <CommCtrl.h>
#include "../paradox/PdxString.h"
#include "NstStatusBar.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

STATUSBAR::STATUSBAR()
: 
hParent (NULL),
hWnd    (NULL)
{
	PDX_COMPILE_ASSERT( FPS_OFFSET == 5 );
	strcpy( sFPS, "FPS: ");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID STATUSBAR::Create(HINSTANCE hInstance,HWND h)
{
	PDX_ASSERT( hInstance && h );

	if (!hWnd)
	{
		hParent = h;

		hWnd = CreateWindowEx
		( 
			0,                       
			STATUSCLASSNAME,         
			NULL,          
			WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,
			0,0,0,0,
			hParent,
			HMENU(CHILD_ID),
			hInstance,
			NULL
		);

		if (!hWnd)
			throw ("CreateWindowEx() failed!");

		HDC hDC = ::GetDC( hWnd );

		if (hDC)
		{
			TEXTMETRIC tm;
			
			if (::GetTextMetrics( hDC, &tm ))
				widths.character = tm.tmAveCharWidth;

			::ReleaseDC( hWnd, hDC );
		}

		widths.fps = widths.character * 11;

		UpdateParts();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID STATUSBAR::Destroy()
{
	if (hWnd)
	{
		::DestroyWindow( hWnd );
		hWnd = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID STATUSBAR::Resize()
{
	if (hWnd)
	{
		::SendMessage( hWnd, WM_SIZE, 0, 0 );
		UpdateParts();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT STATUSBAR::GetHeight() const
{
	RECT rect;
	
	if (hWnd && ::GetWindowRect( hWnd, &rect ) && rect.bottom >= rect.top)
		return rect.bottom - rect.top;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID STATUSBAR::UpdateParts()
{
	RECT rect;
	::GetClientRect( hParent, &rect );

	const INT width = rect.right - rect.left;

	INT list[2] = {width - widths.fps, -1};
	::SendMessage( hWnd, SB_SETPARTS, WPARAM(2), LPARAM(list) );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID STATUSBAR::DisplayMsg(const CHAR* msg)
{
	if (hWnd)
	{
		if (!msg) msg = "";
		::SendMessage( hWnd, SB_SETTEXT, WPARAM(0|0), LPARAM(msg) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID STATUSBAR::DisplayFPS(const BOOL show,DOUBLE fps)
{
	if (hWnd)
	{
		if (show)
		{
			fps = PDX_CLAMP( fps, 0.0, 999.0 );
			sprintf( sFPS + FPS_OFFSET, "%.1f", fps );
			::SendMessage( hWnd, SB_SETTEXT, WPARAM(1|0), LPARAM(sFPS) );
		}
		else
		{
			::SendMessage( hWnd, SB_SETTEXT, WPARAM(1|0), LPARAM("") );
		}
	}
}
