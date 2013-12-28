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

#ifndef NST_OBJECT_RAW_H
#define NST_OBJECT_RAW_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		namespace Private
		{
			template<typename T,typename C> class Raw
			{
				T data;
				uint size;

			public:

				Raw()
				: data(NULL), size(0) {}

				Raw(T d,uint s)
				: data(d), size(s) 
				{
				}

				template<typename U> 
				Raw(U& array)
				: data(array), size(array.Size() * sizeof(array[0]))
				{
				}

				template<typename U,size_t N>
				Raw(U (&array)[N])
				: data(array), size(N * sizeof(U))
				{
				}

				template<typename U>
				Raw& operator = (U& array)
				{
					data = array;
					size = array.Size() * sizeof(array[0]);
					return *this;
				}

				template<typename U,size_t N>
				Raw& operator = (U (&array)[N])
				{
					data = array;
					size = N * sizeof(U);
					return *this;
				}

				void Set(T d,uint s)
				{
					data = d;
					size = s;
				}

				void Reset()
				{
					data = NULL;
					size = 0;
				}

				uint Size() const
				{
					return size;
				}

				ibool Empty() const
				{
					return !size;
				}
  
				template<typename I>
				C& operator [] (I i) const
				{
					NST_ASSERT( size > i );
					return static_cast<C*>(data)[i];
				}

				template<typename I>
				C* operator + (I i) const
				{
					NST_ASSERT( size > i );
					return static_cast<C*>(data) + i;
				}

				template<typename U>
				operator U* () const
				{
					return static_cast<U*>(data);
				}
			};
		}

		typedef Private::Raw<void*,char> Raw;
		typedef Private::Raw<const void*,const char> ConstRaw;
	}
}

#endif
