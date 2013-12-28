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

#include "../NstMapper.hpp"
#include "NstMapper206.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper206::SubReset(const bool hard)
		{
			if (hard)
				command = 0;

			for (dword i=0x8000U; i < 0x10000UL; i += 0x2)
			{
				Map( i + 0x0, &Mapper206::Poke_8000 );
				Map( i + 0x1, &Mapper206::Poke_8001 );
			}
		}
	
		void Mapper206::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					command = state.Read8();
	
				state.End();
			}
		}
	
		void Mapper206::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(Mapper206,8000)
		{
			command = data;
		}
	
		NES_POKE(Mapper206,8001)
		{
			const uint index = command & 0x7;
	
			if (index < 0x6)
			{
				ppu.Update();

				data &= 0x3F;

				if (index < 0x2)
					chr.SwapBank<SIZE_2K>( index << 11, data >> 1 );
				else
					chr.SwapBank<SIZE_1K>( (index + 0x2) << 10, data );
			}
			else
			{
				prg.SwapBank<SIZE_8K>( (index & 0x1) << 13, data & 0xF );
			}
		}
	}
}
