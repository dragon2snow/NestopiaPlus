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

#ifndef NST_DIRECTX_H
#define NST_DIRECTX_H

#pragma once

#include <vector>
#include "NstObjectPointer.hpp"
#include "NstSystemGuid.hpp"

namespace Nestopia
{
	namespace DirectX
	{
		struct BaseAdapter
		{
			ibool operator == (const BaseAdapter& a)  const { return guid == a.guid; }
			ibool operator <  (const BaseAdapter& a)  const { return guid < a.guid;  }
			ibool operator == (const System::Guid& g) const { return guid == g;      }

			System::Guid guid;
			HeapString name;
		};

		template<typename T>
		class ComInterface : public Object::Pointer<T>
		{
		public:

			ulong Release();

			T* operator = (T*);

			ComInterface() {}

			explicit ComInterface(T* p)
			: Object::Pointer<T>(p)
			{
				if (p)
					p->AddRef();
			}

			ComInterface(const ComInterface& ref)
			: Object::Pointer<T>(ref.pointer)
			{
				if (ref.pointer)
					ref.pointer->AddRef();
			}

			~ComInterface()
			{
				if (this->pointer)
					this->pointer->Release();
			}

			operator T* () const
			{
				return this->pointer;
			}

			T* operator = (const ComInterface& ref)
			{
				return *this = ref.pointer;
			}

			ibool operator == (const T* p) const
			{
				return this->pointer == p;
			}

			ibool operator != (const T* p) const
			{
				return this->pointer != p;
			}

			T** operator & ()
			{
				NST_ASSERT( !this->pointer );
				return &this->pointer;
			}

			bool operator ! () const
			{
				return !this->pointer;
			}
		};

		template<typename T>
		T* ComInterface<T>::operator = (T* p)
		{
			if (this->pointer != p)
			{
				if (this->pointer)
					this->pointer->Release();

				if (p)
					p->AddRef();

				this->pointer = p;
			}

			return p;
		}

		template<typename T>
		ulong ComInterface<T>::Release()
		{
			if (T* const tmp = this->pointer)
			{
				this->pointer = NULL;
				return tmp->Release();
			}

			return 0;
		}
	}
}

#endif
