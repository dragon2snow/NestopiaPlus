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

#ifndef NST_MEMORY_H
#define NST_MEMORY_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class LinearMemory
		{
		public:

			LinearMemory(dword);
			LinearMemory(const LinearMemory&);

			void operator =  (const LinearMemory&);
			void operator += (const LinearMemory&);

			void Set(dword);
			void Arrange(dword);
			void Destroy();
			void Clear();

		private:

			static dword PowerUp(dword);

			u8* mem;
			dword size;

		public:

			LinearMemory()
			: mem(NULL), size(0) {}

			~LinearMemory()
			{ 
				delete [] mem; 
			}

			u8* Mem(dword offset=0)
			{
				NST_ASSERT( offset <= size );
				return mem + offset;
			}

			const u8* Mem(dword offset=0) const
			{
				NST_ASSERT( offset <= size );
				return mem + offset;
			}

			u8& operator [] (dword offset)
			{
				NST_ASSERT( offset < size );
				return mem[offset];
			}

			const u8& operator [] (dword offset) const
			{
				NST_ASSERT( offset < size );
				return mem[offset];
			}

			dword Size() const
			{ 
				return size; 
			}

			dword ActualSize() const
			{
				return PowerUp( size );
			}

			bool Empty() const
			{ 
				return size == 0; 
			}
		};
	}
}

#endif
