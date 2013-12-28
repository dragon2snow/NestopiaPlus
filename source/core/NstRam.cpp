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

#include <cstdlib>
#include <cstring>
#include "NstCore.hpp"
#include "NstRam.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		Ram::Ram()
		:
		mem      ( NULL  ),
		mask     ( 0     ),
		size     ( 0     ),
		writable ( false ),
		readable ( false ),
		internal ( false ),
		pad      ( 0     )
		{}

		Ram::~Ram()
		{
			if (internal)
				std::free( mem );
		}

		void Ram::Destroy()
		{
			mask = 0;
			size = 0;

			if (u8* const tmp = mem)
			{
				mem = NULL;

				if (internal)
				{
					internal = false;
					std::free( tmp );
				}
			}
		}

		void Ram::Set(dword s,u8* m)
		{
			NST_ASSERT( s != 1 );

			if (s)
			{
				mask = s - 1;
				mask |= mask >> 1;
				mask |= mask >> 2;
				mask |= mask >> 4;
				mask |= mask >> 8;
				mask |= mask >> 16;

				size = s;

				if (m)
				{
					NST_VERIFY( s == mask+1 );

					if (internal)
					{
						internal = false;
						std::free( mem );
					}
				}
				else
				{
					m = static_cast<u8*>(std::realloc( internal ? mem : NULL, mask+1 ));

					if (m)
					{
						internal = true;
					}
					else
					{
						Destroy();
						throw RESULT_ERR_OUT_OF_MEMORY;
					}
				}

				mem = m;
			}
			else
			{
				Destroy();
			}
		}

		void Ram::Set(bool r,bool w,dword s,u8* m)
		{
			Set( s, m );
			readable = r;
			writable = w;
		}

		void Ram::Fill(const uint value)
		{
			NST_ASSERT( bool(mem) == bool(size) );
			std::memset( mem, value, size );
		}

		void Ram::Mirror(dword block)
		{
			if (dword next = size)
			{
				if (block > next)
					block = next;

				for (const dword begin=next-block, end=mask+1; next < end; next += block)
					std::memcpy( mem + next, mem + begin, NST_MIN(end-next,block) );
			}
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
