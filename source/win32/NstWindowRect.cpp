////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#include "NstWindowRect.hpp"

namespace Nestopia
{
	Window::Rect::Screen::Screen(HWND const hWnd)
	{
		::GetClientRect( hWnd, this );
		::ClientToScreen( hWnd, reinterpret_cast<POINT*>( &left  ) );
		::ClientToScreen( hWnd, reinterpret_cast<POINT*>( &right ) );
	}

	Window::Rect::Window::Window(HWND const hWnd)
	{
		::GetWindowRect( hWnd, this );
	}

	Window::Rect::Client::Client(HWND const hWnd)
	{
		::GetClientRect( hWnd, this );
	}

	Window::Rect::Picture::Picture(HWND const hWnd,const Space space)
	{
		::GetClientRect( hWnd, this );

		for (HWND hChild = ::GetTopWindow(hWnd); hChild; hChild = ::GetNextWindow(hChild,GW_HWNDNEXT))
		{
			RECT rChild;
			::GetClientRect( hChild, &rChild );
			bottom -= rChild.bottom - rChild.top;
		}

		if (bottom < 0)
			bottom = 0;

		if (space == SCREEN)
		{
			::ClientToScreen( hWnd, reinterpret_cast<POINT*>( &left  ) );
			::ClientToScreen( hWnd, reinterpret_cast<POINT*>( &right ) );
		}
	}
}
