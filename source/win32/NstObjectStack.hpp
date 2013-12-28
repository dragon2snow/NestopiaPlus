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

#ifndef NST_OBJECT_STACK_H
#define NST_OBJECT_STACK_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		template<typename T> class Stack : Sealed
		{
			union
			{
				ulong somealignment;
				char data[sizeof(T)];
			};

			ibool valid;

		public:

			void Destruct();

			Stack()
			: valid(FALSE) {}

			~Stack()
			{
				Destruct();
			}

			operator ibool () const
			{
				return valid;
			}

			T* operator -> ()
			{
				NST_ASSERT( valid );
				return reinterpret_cast<T*>(data);
			}

			const T* operator -> () const
			{
				NST_ASSERT( valid );
				return reinterpret_cast<const T*>(data);
			}

			T& operator * ()
			{
				NST_ASSERT( valid );
				return *reinterpret_cast<T*>(data);
			}

			const T& operator * () const
			{
				NST_ASSERT( valid );
				return *reinterpret_cast<const T*>(data);
			}

			void Validate()
			{
				valid = TRUE;
			}

			void Invalidate()
			{
				valid = FALSE;
			}

			void Construct()
			{
				Destruct();
				new (static_cast<void*>(data)) T;
				valid = TRUE;
			}

			template<typename A>
			void Construct(A& a)
			{
				Destruct();
				new (static_cast<void*>(data)) T( a );
				valid = TRUE;
			}

			template<typename A,typename B>
			void Construct(A& a,B& b)
			{
				Destruct();
				new (static_cast<void*>(data)) T( a, b );
				valid = TRUE;
			}

			template<typename A,typename B,typename C>
			void Construct(A& a,B& b,C& c)
			{
				Destruct();
				new (static_cast<void*>(data)) T( a, b, c );
				valid = TRUE;
			}

			template<typename A,typename B,typename C,typename D>
			void Construct(A& a,B& b,C& c,D& d)
			{
				Destruct();
				new (static_cast<void*>(data)) T( a, b, c, d );
				valid = TRUE;
			}

			template<typename A,typename B,typename C,typename D,typename E>
			void Construct(A& a,B& b,C& c,D& d,E& e)
			{
				Destruct();
				new (static_cast<void*>(data)) T( a, b, c, d, e );
				valid = TRUE;
			}

			template<typename A,typename B,typename C,typename D,typename E,typename F>
			void Construct(A& a,B& b,C& c,D& d,E& e,F& f)
			{
				Destruct();
				new (static_cast<void*>(data)) T( a, b, c, d, e, f );
				valid = TRUE;
			}
		};

		template<typename T>
		void Stack<T>::Destruct()
		{
			if (valid)
			{
				valid = FALSE;
				reinterpret_cast<T*>(data)->T::~T();
			}
		}
	}
}

#endif
