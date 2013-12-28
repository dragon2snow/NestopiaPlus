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

#ifndef NST_WINDOW_PARAM_H
#define NST_WINDOW_PARAM_H

#pragma once

#include <cstdlib>
#include "NstWindowGeneric.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Param
		{
			WPARAM wParam;
			LPARAM lParam;
			LRESULT lResult;
			HWND hWnd;

			class ButtonParam
			{
				friend struct Param;

				const uint wParam;

				explicit ButtonParam(uint w)
				: wParam(w) {}

			public:

				ibool IsClicked() const
				{
					return HIWORD(wParam) == BN_CLICKED;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class ListBoxParam
			{
				friend struct Param;

				const uint wParam;

				explicit ListBoxParam(uint w)
				: wParam(w) {}

			public:

				ibool SelectionChanged() const
				{
					return HIWORD(wParam) == LBN_SELCHANGE;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class ComboBoxParam
			{
				friend struct Param;

				const uint wParam;

				explicit ComboBoxParam(uint w)
				: wParam(w) {}

			public:

				ibool SelectionChanged() const
				{
					return HIWORD(wParam) == CBN_SELCHANGE;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class EditParam
			{
				friend struct Param;

				const uint wParam;

				explicit EditParam(uint w)
				: wParam(w) {}

			public:

				ibool SetFocus() const
				{
					return HIWORD(wParam) == EN_SETFOCUS;
				}

				ibool KillFocus() const
				{
					return HIWORD(wParam) == EN_KILLFOCUS;
				}

				ibool Changed() const
				{
					return HIWORD(wParam) == EN_CHANGE;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class SliderParam
			{
				friend struct Param;

				const Param& param;

				explicit SliderParam(const Param& p)
				: param(p) {}

			public:

				uint Scroll() const;

				ibool IsControl(uint id) const
				{
					return reinterpret_cast<HWND>(param.lParam) == ::GetDlgItem( param.hWnd, id );
				}

				ibool Released() const
				{
					return LOWORD(param.wParam) == SB_ENDSCROLL;
				}
			};

			class CursorParam
			{
				friend struct Param;

				const Param& param;

				explicit CursorParam(const Param& p)
				: param(p) {}
               
			public:

				ibool IsInside(uint id) const
				{
					return reinterpret_cast<HWND>(param.wParam) == ::GetDlgItem( param.hWnd, id ); 
				}
			};

			class ActivatorParam
			{
				friend struct Param;

				const Param& param;

				explicit ActivatorParam(const Param& p)
				: param(p) {}

			public:

				ibool IsLeaving() const
				{
					return LOWORD(param.wParam) == WA_INACTIVE;
				}

				ibool IsEntering() const
				{
					return !IsLeaving();
				}

				ibool IsInsideApplication() const
				{
					return param.lParam && ::IsChild( param.hWnd, reinterpret_cast<HWND>(param.lParam) );
				}

				ibool IsOutsideApplication() const
				{
					return !IsInsideApplication();
				}

				ibool Minimized() const
				{
					return HIWORD(param.wParam);
				}
			};

			ButtonParam    Button()    const { return ButtonParam    ( wParam ); }
			ListBoxParam   ListBox()   const { return ListBoxParam   ( wParam ); }
			ComboBoxParam  ComboBox()  const { return ComboBoxParam  ( wParam ); }
			EditParam      Edit()      const { return EditParam      ( wParam ); }
			SliderParam    Slider()    const { return SliderParam    ( *this  ); }
			CursorParam    Cursor()    const { return CursorParam    ( *this  ); }
			ActivatorParam Activator() const { return ActivatorParam ( *this  ); }
			Generic        Window()    const { return Generic        ( hWnd   ); }
		};
	}
}

#endif
