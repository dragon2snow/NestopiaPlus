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

#ifndef NST_IO_SCREEN_H
#define NST_IO_SCREEN_H

#pragma once

#include "NstObjectDelegate.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class Video;
	}

	namespace Io
	{
		class Screen : HeapString
		{
			friend class Managers::Video;

			typedef HeapString Text;
			typedef Object::Delegate<void,const GenericString&> Callback;

			Screen(const Screen&);
			void operator = (const Screen&);

			static Callback callback;

		public:

			Screen() {}
			~Screen();

			template<typename T>
			Screen& operator << (const T& t)
			{
				Text::operator << (t);
				return *this;
			}
		};
	}
}

#endif
