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

#include "../NstMapper.hpp"
#include "NstMapper226.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper226::SubReset(const bool hard)
		{
			if (hard)
				regs[1] = regs[0] = 0;

			Map( 0x8000U, 0xFFFFU, &Mapper226::Poke_Prg );
		}
	
		void Mapper226::SubLoad(State::Loader& state)
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
	
		void Mapper226::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write16( regs[0] | (regs[1] << 8) ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper226,Prg) 
		{
			regs[address & 0x1] = data;
	
			uint bank = 
			(
				((regs[0] & 0x1E) >> 1) |
				((regs[0] & 0x80) >> 3) |
				((regs[1] & 0x01) << 5)
			);
	
			if (regs[0] & 0x20)
			{
				bank = (bank << 1) | (regs[0] & 0x1);
				prg.SwapBanks<NES_16K,0x0000U>( bank, bank );
			}
			else
			{
				prg.SwapBank<NES_32K,0x0000U>( bank );
			}
	
			ppu.SetMirroring( (regs[0] & 0x40) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
		}
	}
}
