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
	:
	data     (inSize ? std::malloc( inSize ) : NULL),
	capacity (inSize),
	size     (inSize)
	{}

	Base::Base(const void* const NST_RESTRICT inData,const uint inSize)
	:
	data     (inSize ? std::memcpy( std::malloc( inSize ), inData, inSize ) : NULL),
	capacity (inSize),
	size     (inSize)
	{
		NST_ASSERT( bool(inData) >= bool(inSize) );
	}

	Base::Base(const Base& base)
	:
	data     (base.size ? std::memcpy( std::malloc( base.size ), base.data, base.size ) : NULL),
	capacity (base.size),
	size     (base.size)
	{
	}

	ibool Base::Valid(const void* const it) const
	{
		return it >= bytes && it <= bytes + size;
	}

	ibool Base::InBound(const void* const it) const
	{
		return it >= bytes && it < bytes + size;
	}

	void Base::Assign(const void* const NST_RESTRICT inData,const uint inSize)
	{
		size = inSize;

		if (capacity < inSize)
			data = std::realloc( data, capacity = inSize );

		std::memcpy( data, inData, inSize );
	}

	void Base::Append(const void* const NST_RESTRICT inData,const uint inSize)
	{
		size += inSize;

		if (capacity < size)
			data = std::realloc( data, capacity = size * 2 );

		std::memcpy( bytes + (size - inSize), inData, inSize );
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

				void* const NST_RESTRICT next = std::malloc( capacity );

				std::memcpy( next, data, pos );

				if (inData)
					std::memcpy( static_cast<char*>(next) + pos, inData, inSize );

				std::memcpy( static_cast<char*>(next) + pos + inSize, offset, end );

				void* tmp = data;
				data = next;
				std::free( tmp );
			}
		}
	}

	void Base::Erase(void* const begin,void* const end)
	{
		NST_ASSERT( end >= begin && begin >= bytes && end <= bytes + size );

		const uint back = (bytes + size) - static_cast<char*>(end);
		size -= static_cast<char*>(end) - static_cast<char*>(begin);

		std::memmove( begin, end, back );
	}

	void Base::Destroy()
	{
		if (void* tmp = data)
		{
			data = NULL;
			capacity = 0;
			size = 0;
			std::free( tmp );
		}
	}

	void Base::operator = (const Base& base)
	{
		Assign( base.data, base.size );
	}

	void Base::Reserve(const uint inSize)
	{
		if (capacity < inSize)
			data = std::realloc( data, capacity = inSize );
	}

	void Base::Resize(const uint inSize)
	{
		size = inSize;
		Reserve( inSize );
	}

	void Base::Grow(const uint inSize)
	{
		Resize( size + inSize );
	}

	void Base::Defrag()
	{
		if (capacity != size)
			data = std::realloc( data, capacity = size );
	}

	void Base::Import(Base& base)
	{
		void* tmp = data;
		data = base.data;
		base.data = NULL;
		size = base.size;
		base.size = 0;
		capacity = base.capacity;
		base.capacity = 0;
		std::free( tmp );
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
