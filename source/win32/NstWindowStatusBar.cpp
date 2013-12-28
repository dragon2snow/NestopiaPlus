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

#include "NstApplicationException.hpp"
#include "NstWindowCustom.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowRect.hpp"
#include "NstWindowStatusBar.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	using namespace Window;

	inline StatusBar::Width::Width(const uint n)
	: numChars(n) {}

	void StatusBar::Width::Calculate(HWND const hWnd)
	{
		NST_ASSERT( hWnd );

		character = DEF_CHAR_WIDTH;
		first = DEF_FIRST_WIDTH;

		if (HDC const hDC = ::GetDC( hWnd ))
		{
			TEXTMETRIC tm;

			if (::GetTextMetrics( hDC, &tm ))
				character = tm.tmAveCharWidth;

			first = character * numChars;
			
			::ReleaseDC( hWnd, hDC );
		}
	}

	StatusBar::StatusBar(Custom& p,const uint numChars)
	: 
	parent ( p ),
	width  ( numChars ) 
	{
		static const MsgHandler::HookEntry<StatusBar> hooks[] =
		{
			{ WM_SIZE,    &StatusBar::OnSize    },
			{ WM_DESTROY, &StatusBar::OnDestroy }
		};

		p.Messages().Hooks().Add( this, hooks );
	}

	StatusBar::~StatusBar()
	{
		window.Destroy();
	}

	void StatusBar::Enable(const ibool enable,const ibool show)
	{
		NST_ASSERT( bool(parent) );

		if (enable)
		{
			if (window == NULL)
			{
				window = Generic::Create
				(
					0,                       
					STATUSCLASSNAME,         
					NULL,          
					WS_CHILD|SBARS_SIZEGRIP|(show ? WS_VISIBLE : 0),
					0,0,0,0,
					parent,
					reinterpret_cast<HMENU>(CHILD_ID),
					NULL
				);

				width.Calculate( window );
				Update();
			}
		}
		else
		{
			window.Destroy();
		}
	}

	void StatusBar::OnDestroy(Param&)
	{
		window.Destroy();
	}

	void StatusBar::Show() const
	{
		window.Show( SW_SHOWNA );
	}

	void StatusBar::Stream::operator << (tstring text) const
	{
		if (window)
			window.Send( SB_SETTEXT, field|0, text );
	}

	void StatusBar::Stream::Clear() const
	{
		operator << (_T(""));
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	uint StatusBar::GetMaxMessageLength() const
	{
		if (window)
		{
			int list[2] = {0,0};
			window.Send( SB_GETPARTS, 2, list );

			if (list[0] > int(width.character)) 
				return uint(list[0]) / (width.character - 1);
		}

		return 0;
	}

	void StatusBar::Update() const
	{
		NST_ASSERT( window && parent );

		int list[2] = { Rect::Client( parent ).Width() - (int) width.first, -1 };
		window.Send( SB_SETPARTS, 2, list );
	}

	void StatusBar::OnSize(Param& param)
	{
		if (window)
		{
			window.Send( WM_SIZE, param.wParam, param.lParam );
			Update();
		}
	}

	uint StatusBar::GetHeight() const
	{
		if (window)
		{
			const int height = Rect::Window( window ).Height();

			if (height > 0)
				return height;
		}

		return 0;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif
}

