////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include "NstWindowParam.hpp"
#include <CommCtrl.h>
#include <ShellAPI.h>

namespace Nestopia
{
	using Window::Param;

	uint Param::SliderParam::Scroll() const
	{
		switch (LOWORD(param.wParam))
		{
       		case SB_THUMBTRACK:
     		case SB_THUMBPOSITION:
			return HIWORD(param.wParam);
		}

		return ::SendMessage( reinterpret_cast<HWND>(param.lParam), TBM_GETPOS, 0, 0 ); 
	}

	ibool Param::DropFilesParam::IsInside(HWND const hWnd) const
	{
		Point point;
		::DragQueryPoint( reinterpret_cast<HDROP>(param.wParam), &point );
		::ClientToScreen( param.hWnd, &point );
		return Rect::Window( hWnd ).IsInside( point );
	}

	uint Param::DropFilesParam::Size() const
	{
		return ::DragQueryFile( reinterpret_cast<HDROP>(param.wParam), 0xFFFFFFFF, NULL, 0 );
	}

	Path Param::DropFilesParam::operator [] (const uint i) const
	{		
		Path file;

		if (const uint length = ::DragQueryFile( reinterpret_cast<HDROP>(param.wParam), i, NULL, 0 ))
		{
			file.Resize( length );
			::DragQueryFile( reinterpret_cast<HDROP>(param.wParam), i, file.Ptr(), length + 1 );
		}

		return file;
	}
}
