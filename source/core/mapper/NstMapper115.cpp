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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper115.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper115::SubReset(const bool hard)
		{
			if (hard)
			{
				exRegs[0] = 0;
				exRegs[1] = 0;
			}

			Mmc3::SubReset( hard );
	
			Map( 0x6000U, &Mapper115::Poke_6000 );
			Map( 0x6001U, &Mapper115::Poke_6001 );
		}
	
		void Mapper115::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<2> data( state );
					exRegs[0] = data[0];
					exRegs[1] = data[1];
				}
	
				state.End();
			}
		}
	
		void Mapper115::SubSave(State::Saver& state) const
		{
			const u8 data[2] =
			{
				exRegs[0],
				exRegs[1]
			};

			state.Begin('R','E','G','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper115::UpdatePrg()
		{
			Mmc3::UpdatePrg();

			if (exRegs[0] & 0x80)
				prg.SwapBank<NES_16K,0x0000U>( exRegs[0] & 0x07 );
		}

		void Mapper115::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;
			uint high = (exRegs[1] & 0x1) << 7;

			chr.SwapBanks<NES_2K>
			( 
		    	0x0000U ^ swap, 
				banks.chr[0] | high, 
				banks.chr[1] | high 
			); 
			
			high <<= 1;

			chr.SwapBanks<NES_1K>
			( 
		     	0x1000U ^ swap, 
				banks.chr[2] | high, 
				banks.chr[3] | high, 
				banks.chr[4] | high, 
				banks.chr[5] | high 
			); 
		}

		NES_POKE(Mapper115,6000)
		{
			if (exRegs[0] != data)
			{
				exRegs[0] = data;
				Mapper115::UpdatePrg();
			}
		}
	
		NES_POKE(Mapper115,6001)
		{
			if (exRegs[1] != data)
			{
				exRegs[1] = data;
				Mapper115::UpdateChr();
			}
		}
	}
}
