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

#include <cstring>
#include "NstCollectionVector.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("t", on)
#endif

namespace Nestopia
{
	using Collection::Private::Base;

	Base::Base(const uint inSize)
	: data(inSize ? operator new ((inSize + 4) & ~3U) : NULL), capacity(inSize), size(inSize) {}

	Base::Base(const void* const NST_RESTRICT inData,const uint inSize)
	: data(NULL), capacity(inSize), size(inSize)
	{
		NST_ASSERT( bool(inData) >= bool(inSize) );

		if (inSize)
		{
			data = operator new ((inSize + 4) & ~3U);
			std::memcpy( data, inData, inSize );
		}
	}

	Base::Base(const Base& base)
	: data(NULL), capacity(base.size), size(base.size)
	{
		if (base.size)
		{
			data = operator new ((base.size + 4) & ~3U);
			std::memcpy( data, base.data, (base.size + 4) & ~3U );
		}
	}

	ibool Base::Valid(const void* const it) const throw()
	{
		return it >= bytes && it <= bytes + size;
	}

	ibool Base::InBound(const void* const it) const throw()
	{
		return it >= bytes && it < bytes + size;
	}

	void Base::Allocate(const uint inSize)
	{
		NST_ASSERT( inSize );

		operator delete (data);
		data = NULL;
		capacity = inSize;
		data = operator new ((inSize + 4) & ~3U);
	}

	void Base::Reallocate(const uint inSize)
	{
		NST_ASSERT( inSize );

		if (size)
		{
			capacity = inSize;
			void* next = operator new ((inSize + 4) & ~3U);
			std::memcpy( next, data, (size + 4) & ~3U );	

			void* tmp = data;
			data = next;
			operator delete (tmp);
		}
		else
		{
			Allocate( inSize );
		}
	}

	void Base::Assign(const void* const NST_RESTRICT inData,const uint inSize)
	{
		if (capacity < inSize)
			Allocate( inSize );

		size = inSize;
		std::memcpy( data, inData, inSize );
	}

	void Base::Append(const void* const NST_RESTRICT inData,const uint inSize)
	{
		if (capacity < size + inSize)
			Reallocate( (size + inSize) * 2 );

		void* next = bytes + size; size += inSize;
		std::memcpy( next, inData, inSize );
	}

	void Base::Insert(void* const offset,const void* const NST_RESTRICT inData,const uint inSize)
	{
		NST_ASSERT( Valid(offset) );

		if (inSize)
		{
			const uint pos = static_cast<char*>(offset) - bytes;
			const uint end = size - pos;

			size += inSize;

			if (capacity >= size)
			{
				std::memmove( static_cast<char*>(offset) + inSize, offset, end );

				if (inData)
					std::memcpy( offset, inData, inSize );
			}
			else
			{
				capacity = size * 2;

				void* const NST_RESTRICT next = operator new ((capacity + 4) & ~3U);

				std::memcpy( next, data, pos );

				if (inData)
					std::memcpy( static_cast<char*>(next) + pos, inData, inSize );

				std::memcpy( static_cast<char*>(next) + pos + inSize, offset, end );

				void* const tmp = data;
				data = next;
				operator delete (tmp);
			}
		}
	}

	void Base::Erase(void* const begin,void* const end) throw()
	{
		NST_ASSERT( end >= begin && begin >= bytes && end <= bytes + size );

		const uint back = (bytes + size) - static_cast<char*>(end);
		size -= static_cast<char*>(end) - static_cast<char*>(begin);

		std::memmove( begin, end, back );
	}

	void Base::Destroy() throw()
	{
		if (data)
		{
			void* tmp = data;
			data = NULL;
			capacity = 0;
			size = 0;
			operator delete (tmp);
		}
	}

	void Base::operator = (const Base& base)
	{
		Assign( base.data, base.size );
	}

	void Base::Reserve(const uint inSize)
	{
		if (capacity < inSize)
			Reallocate( inSize );
	}

	void Base::Resize(const uint inSize)
	{
		Reserve( inSize );
		size = inSize;
	}

	void Base::Grow(const uint inSize)
	{
		Resize( size + inSize );
	}

	void Base::Defrag()
	{
		if (capacity != size)
		{
			if (size)
			{
				Reallocate( size );
			}
			else
			{
				operator delete (data);
				data = NULL;
				capacity = 0;
			}
		}
	}

	void Base::Import(void* const inData,const uint inSize) throw()
	{
		operator delete (data);
		data = inData;
		size = capacity = inSize;
	}

	void Base::Import(Base& base) throw()
	{
		operator delete (data);
		data = base.data;
		base.data = NULL;
		size = base.size;
		base.size = 0;
		capacity = base.capacity;
		base.capacity = 0;
	}

	void* Base::Export() throw()
	{
		void* tmp = data;
		data = NULL;
		capacity = 0;
		size = 0;
		return tmp;
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
