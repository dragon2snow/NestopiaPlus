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

#ifndef NST_VECTOR_H
#define NST_VECTOR_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <cstring>

namespace Nes
{
	namespace Core
	{
		class BaseVector
		{
		protected:

			static void* Allocate(dword);
			static void* Reallocate(void*,dword);
			static void  Free(void*);
		};

		template<typename T>
		class Vector : BaseVector
		{
			T* data;
			dword size;
			dword capacity;

			void MakeRoom(dword);
			void MakeMoreRoom(dword);

		public:

			typedef T Type;

			Vector();
			explicit Vector(dword);
			Vector(const Vector&);

			void operator << (const T&);
			bool operator == (const Vector&) const;

			void Reserve(dword);
			void Resize(dword);
			void Expand(dword);
			void Assign(const T*,dword);
			void Append(const T*,dword);
			void Erase(T*,dword=1);
			void Destroy();
			void Defrag();

			static void Swap(Vector&,Vector&);

			~Vector()
			{
				BaseVector::Free( data );
			}

			void operator = (const Vector& vector)
			{
				Assign( vector.data, vector.size );
			}

			void operator += (const Vector& vector)
			{
				Append( vector.data, vector.size );
			}

			T& operator [] (dword i) const
			{
				NST_ASSERT( i < size );
				return data[i];
			}

			T* Begin() const
			{
				return data;
			}

			T* End() const
			{
				return data + size;
			}

			dword Size() const
			{
				return size;
			}

			dword Capacity() const
			{
				return capacity;
			}

			T& Front() const
			{
				NST_ASSERT( size );
				return data[0];
			}

			T& Back() const
			{
				NST_ASSERT( size );
				return data[size - 1];
			}

			const T& Pop()
			{
				NST_ASSERT( size );
				return data[--size];
			}

			void SetTo(dword count)
			{
				NST_ASSERT( count <= capacity );
				size = count;
			}

			void Clear()
			{
				size = 0;
			}
		};

		template<typename T>
		Vector<T>::Vector()
		: data(NULL), size(0), capacity(0) {}

		template<typename T>
		Vector<T>::Vector(const dword count)
		: data(static_cast<T*>(BaseVector::Allocate(count * sizeof(T)))), size(count), capacity(count)
		{
		}

		template<typename T>
		Vector<T>::Vector(const Vector<T>& v)
		: data(static_cast<T*>(BaseVector::Allocate(v.size * sizeof(T)))), size(v.size), capacity(v.size)
		{
			std::memcpy( data, v.data, v.size * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::MakeRoom(const dword count)
		{
			void* const tmp = data;
			data = NULL;
			BaseVector::Free( tmp );
			data = static_cast<T*>(BaseVector::Allocate( (capacity=count) * sizeof(T) ));
		}

		template<typename T>
		void Vector<T>::MakeMoreRoom(const dword count)
		{
			data = static_cast<T*>(BaseVector::Reallocate( data, (capacity=count) * sizeof(T) ));
		}

		template<typename T>
		void Vector<T>::operator << (const T& value)
		{
			if (size == capacity)
				MakeMoreRoom( capacity = (size + 1) * 2 );

			data[size++] = value;
		}

		template<typename T>
		void Vector<T>::Assign(const T* const NST_RESTRICT inData,const dword inSize)
		{
			if (capacity < inSize)
				MakeRoom( inSize );

			std::memcpy( data, inData, (size=inSize) * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::Append(const T* const NST_RESTRICT inData,const dword inSize)
		{
			if (capacity < size + inSize)
				MakeMoreRoom( (size * 2) + inSize );

			void* const tmp = data + size;
			size += inSize;

			std::memcpy( tmp, inData, inSize * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::Erase(T* it,dword count)
		{
			NST_ASSERT( size >= count );

			const dword s = size;
			size -= count;
			std::memmove( it, it + count, (s - (it-data + count)) * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::Reserve(dword count)
		{
			if (capacity < count)
				MakeMoreRoom( count );
		}

		template<typename T>
		void Vector<T>::Resize(dword count)
		{
			size = count;
			Reserve( count );
		}

		template<typename T>
		void Vector<T>::Expand(dword count)
		{
			size += count;
			Reserve( size );
		}

		template<typename T>
		void Vector<T>::Defrag()
		{
			if (size < capacity)
				MakeMoreRoom( size );
		}

		template<typename T>
		bool Vector<T>::operator == (const Vector<T>& vector) const
		{
			return size == vector.size && std::memcmp( data, vector.data, size * sizeof(T) ) == 0;
		}

		template<typename T>
		void Vector<T>::Destroy()
		{
			if (void* const tmp = data)
			{
				data = NULL;
				size = 0;
				capacity = 0;
				BaseVector::Free( tmp );
			}
		}

		template<typename T>
		void Vector<T>::Swap(Vector& a,Vector& b)
		{
			T* t = a.data;
			a.data = b.data;
			b.data = t;
			dword u = a.size;
			a.size = b.size;
			b.size = u;
			u = a.capacity;
			a.capacity = b.capacity;
			b.capacity = u;
		}
	}
}

#endif
