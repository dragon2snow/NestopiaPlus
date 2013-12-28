////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_OBJECT_POINTER_H
#define NST_OBJECT_POINTER_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		template<typename T> class Pointer
		{
		protected:

			T* pointer;

		public:

			Pointer(T* t=NULL)
			: pointer(t) {}

			T* operator = (T* t)
			{
				return pointer = t;
			}

			T& operator * () const
			{
				NST_ASSERT( pointer );
				return *pointer;
			}

			T* operator -> () const
			{
				NST_ASSERT( pointer );
				return pointer;
			}
		};
	}
}

#endif
