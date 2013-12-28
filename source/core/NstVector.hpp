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

#include <cstdlib>
#include <cstring>

namespace Nes
{
	namespace Core
	{
		template<typename T>
		class Vector
		{
			void Allocate(dword);
			void Reallocate(dword);

			T* data;
			dword size;
			dword capacity;

		public:

			typedef T Type;

			Vector();
			explicit Vector(dword);
			Vector(const Vector&);
			~Vector();

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
		: data(count ? static_cast<T*>(std::malloc(count * sizeof(T))) : NULL), size(count), capacity(count)
		{
			if (count && data == NULL)
				throw RESULT_ERR_OUT_OF_MEMORY;
		}

		template<typename T>
		Vector<T>::Vector(const Vector<T>& v)
		: data(v.size ? static_cast<T*>(std::malloc(v.size * sizeof(T))) : NULL), size(v.size), capacity(v.size)
		{
			if (data)
			{
				std::memcpy( data, v.data, capacity * sizeof(T) );
			}
			else if (capacity)
			{
				throw RESULT_ERR_OUT_OF_MEMORY;
			}
		}

		template<typename T>
		Vector<T>::~Vector()
		{
			std::free( data );
		}

		template<typename T>
		void Vector<T>::Allocate(const dword count)
		{
			std::free( data );
			data = NULL;

			if (0 < (capacity = count) && NULL == (data = static_cast<T*>(std::malloc( capacity * sizeof(T) ))))
				throw RESULT_ERR_OUT_OF_MEMORY;
		}

		template<typename T>
		void Vector<T>::Reallocate(const dword count)
		{
			capacity = count;

			if (void* const tmp = (data ? std::realloc( data, capacity * sizeof(T) ) : std::malloc( capacity * sizeof(T) )))
				data = static_cast<T*>(tmp);
			else
				throw RESULT_ERR_OUT_OF_MEMORY;
		}

		template<typename T>
		void Vector<T>::operator << (const T& value)
		{
			if (size == capacity)
				Reallocate( (size + 1) * 2 );

			data[size++] = value;
		}

		template<typename T>
		void Vector<T>::Assign(const T* const inData,const dword inSize)
		{
			if (capacity < inSize)
				Allocate( inSize );

			std::memcpy( data, inData, (size=inSize) * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::Append(const T* const inData,const dword inSize)
		{
			if (capacity < size + inSize)
				Reallocate( (size * 2) + inSize );

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
				Reallocate( count );
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
				Reallocate( size );
		}

		template<typename T>
		bool Vector<T>::operator == (const Vector<T>& vector) const
		{
			return size == vector.size && std::memcmp( data, vector.data, size * sizeof(T) ) == 0;
		}

		template<typename T>
		void Vector<T>::Destroy()
		{
			if (T* const tmp = data)
			{
				data = NULL;
				size = 0;
				capacity = 0;
				std::free( tmp );
			}
		}
	}
}

#endif
