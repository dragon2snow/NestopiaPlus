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

#include "NstCore.hpp"
#include "NstCpu.hpp"
#include "NstSoundRenderer.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			Buffer::Buffer(uint bits) 
			{ 
				Reset( bits, true );
			}
		
			void Buffer::Reset(uint bits,const bool clear)
			{
				pos = start = 0;
				history.pos = 0;
		
				bits = (bits == 16 ? 0 : 0x80);
		
				for (uint i=0; i < History::SIZE; ++i)
					history.buffer[i] = bits;
		
				if (clear)
				{
					for (uint i=0; i < SIZE; ++i)
						output[i] = 0;
				}
			}
		
			uint Buffer::Latency() const
			{
				return (pos >= start) ? (pos - start) : (SIZE - start) + pos;
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void Buffer::operator >> (Block& block)
			{
				NST_ASSERT( block.length );
		
				idword delta = pos - start;
		
				if (delta < 0)
					delta += (idword) SIZE;
		
				block.data = output;
				block.start = start;
		
				if (block.length > (uint) delta)
					block.length = delta;
		
				start = (start + block.length) & MASK;
		
				if (start == pos)
					start = pos = 0;
			}
		}
	}
}
