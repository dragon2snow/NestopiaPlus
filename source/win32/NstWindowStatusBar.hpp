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

#ifndef NST_WINDOW_STATUSBAR_H
#define NST_WINDOW_STATUSBAR_H

#pragma once

#include "NstWindowGeneric.hpp"

namespace Nestopia
{
	namespace Window
	{
		class StatusBar
		{
		public:

			StatusBar(Custom&,uint);
			~StatusBar();

			void Enable(ibool=true,ibool=true);
			void Show() const;
			uint GetHeight() const;
			uint GetMaxMessageLength() const;

			enum
			{
				FIRST_FIELD,
				SECOND_FIELD
			};

		private:

			enum {CHILD_ID=1000};

			void Update() const;
			void OnSize(Param&);
			void OnDestroy(Param&);

			struct Width
			{
				enum
				{
					DEF_CHAR_WIDTH = 7,
					DEF_FIRST_WIDTH = 10
				};

				inline Width(uint);

				void Calculate(HWND);

				const uint numChars;
				uint character;
				uint first;
			};

			Width width;
			const Custom& parent;
			Generic window;

		public:

			Generic GetWindow() const
			{
				return window;
			}

			ibool IsEnabled() const
			{
				return window != NULL;
			}

			void Disable()
			{
				Enable( false );
			}

			ibool Toggle()
			{
				Enable( !IsEnabled() );
				return IsEnabled();
			}

			class Stream
			{
				friend class StatusBar;

				const Generic window;
				const uint field;

				Stream(Generic w,uint f)
				: window(w), field(f) {}

			public:

				void operator << (tstring) const;
				void Clear() const;
			};

			Stream Text(uint field=0) const
			{
				return Stream( window, field );
			}
		};
	}
}

#endif
