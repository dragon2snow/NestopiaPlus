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
#include "NstMapper132.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper132::SubReset(const bool hard)
		{
			if (hard)
				regs[1] = regs[0] = 0;

			Map( 0x4100U,          &Mapper132::Peek_4100 );
			Map( 0x4102U, 0x4103U, &Mapper132::Poke_4102 );
			Map( 0x8000U, 0xFFFFU, &Mapper132::Poke_8000 );
		}
	
		void Mapper132::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<2> data( state );
					
					regs[0] = data[0];
					regs[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper132::SubSave(State::Saver& state) const
		{
			const u8 data[2] = { regs[0], regs[1] };
			state.Begin('R','E','G','\0').Write( data ).End();
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_PEEK(Mapper132,4100) 
		{
			return regs[1] ? regs[0] : 0x41;
		}

		NES_POKE(Mapper132,4102) 
		{
			regs[address - 0x4102U] = data;
		}

		NES_POKE(Mapper132,8000) 
		{
			ppu.Update();
			prg.SwapBank<SIZE_32K,0x0000U>( regs[0] >> 2 & 0x1 );
			chr.SwapBank<SIZE_8K,0x0000U>( regs[0] & 0x3 );
		}
	}
}
