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
#include "NstCore.hpp"
#include "NstMemory.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		LinearMemory::LinearMemory(dword s)
		: mem(NULL), size(0)
		{ 
			Set( s ); 
		}

		LinearMemory::LinearMemory(const u8* m,dword s)
		: mem(NULL), size(0)
		{ 
			Set( s ); 
			std::memcpy( mem, m, s );
		}

		LinearMemory::LinearMemory(const LinearMemory& m)
		: mem(NULL), size(0)
		{ 
			operator = (m); 
		}

		void LinearMemory::operator = (const LinearMemory& m)
		{
			Set( m.size );
			std::memcpy( mem, m.mem, m.size );
		}

		void LinearMemory::operator += (const LinearMemory& m)
		{
			const dword end = size;
			Set( end + m.size );
			std::memcpy( mem + end, m.mem, m.size );
		}

		void LinearMemory::Destroy()
		{ 
			u8* const tmp = mem;
			mem = NULL;
			size = 0;
			delete [] tmp;
		}

		dword LinearMemory::PowerUp(dword i)
		{
			--i;		
			i |= i >> 1;
			i |= i >> 2;
			i |= i >> 4;
			i |= i >> 8;
			i |= i >> 16;
			++i;

			return i;
		}

		void LinearMemory::Set(dword s)
		{
			if (size != s)
			{
				if (s)
				{
					const dword ps = PowerUp(s);

					if (size)
					{
						if (PowerUp(size) != ps)
						{
							u8* const m = new u8 [ps];
							std::memcpy( m, mem, NST_MIN(size,s) );
							delete [] mem;
							mem = m;
						}

						size = s;
					}
					else
					{
						size = s;
						mem = new u8 [ps];
					}
				}
				else if (size)
				{
					Destroy();
				}
			}
		}

		void LinearMemory::Arrange(dword block)
		{
			if (const dword end = PowerUp(size))
			{
				if (block > size)
					block = size;

				const dword begin = size - block;

				for (dword next=size; next < end; next += block)
					std::memcpy( mem + next, mem + begin, NST_MIN(end-next,block) );
			}
		}

		void LinearMemory::Clear()
		{
			std::memset( mem, 0x00, size );
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
