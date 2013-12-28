////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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
#include "NstState.hpp"
#include "NstChip.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		dword BaseMemory::Linear::GetPower2(dword i)
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

		BaseMemory::Linear::Linear()
		: 
		mem      ( NULL  ), 
		mask     ( 0     ), 
		size     ( 0     ),
		writable ( false ),
		readable ( false ),
		internal ( false ),
		pad      ( 0     )
		{}

		BaseMemory::Linear::~Linear()
		{
			if (internal)
				delete [] mem;
		}

		void BaseMemory::Linear::Destroy()
		{
			u8* const old = mem;

			mem = NULL;
			mask = 0;
			size = 0;

			if (internal)
			{
				internal = false;
				delete [] old;
			}
		}

		void BaseMemory::Linear::Set(u8* m,dword s)
		{
			NST_ASSERT( m && s );

			u8* const old = mem;

			mem = m;
			size = s;
			mask = GetPower2( s ) - 1;

			if (internal)
			{
				internal = false;
				delete [] old;
			}
		}

		void BaseMemory::Linear::Set(dword s)
		{
			NST_ASSERT( s );

			if (internal)
			{
				if (size == s)
					return;

				delete [] mem;
			}

			internal = true;
			size = s;
			mask = GetPower2( s ) - 1;
			mem = NULL;
			mem = new u8 [mask+1];
		}

		void BaseMemory::Linear::Fill(const int value)
		{
			NST_ASSERT( mem && size );
			std::memset( mem, value, size );
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
