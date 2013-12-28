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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper044.hpp"
	  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper44::SubReset(const bool hard)
		{
			if (hard)
				exReg = 0;

			Mmc3::SubReset( hard );
	
			for (uint i=0xA001U; i < 0xC000U; i += 0x2)
				Map( i, &Mapper44::Poke_A001 );
		}
	
		void Mapper44::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					exReg = state.Read8();
					exReg = NST_MIN(exReg & 7,6);
				}
	
				state.End();
			}
		}
	
		void Mapper44::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( exReg ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper44,A001)
		{
			exReg = NST_MIN(data & 7,6);
	
			Mapper44::UpdatePrg();
			Mapper44::UpdateChr();
		}
	
		void Mapper44::UpdatePrg()
		{
			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) << 8;
			const uint bank = exReg << 4;
			const uint mask = (exReg == 6) ? 0x1F : 0x0F;
	
			prg.SwapBank<SIZE_8K>( 0x0000U ^ swap, bank | (banks.prg[0] & mask) );
			prg.SwapBank<SIZE_8K>( 0x2000U,        bank | (banks.prg[1] & mask) );
			prg.SwapBank<SIZE_8K>( 0x4000U ^ swap, bank | (mask-1)              );
			prg.SwapBank<SIZE_8K>( 0x6000U,        bank | mask                  );
		}
	
		void Mapper44::UpdateChr() const
		{
			ppu.Update();
	
			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;
			const uint bank = exReg << 7;
			const uint mask = (exReg == 6) ? 0xFF : 0x7F;
	
			chr.SwapBanks<SIZE_2K>
			( 
		       	0x0000U ^ swap, 
				(bank >> 1) | (banks.chr[0] & (mask >> 1)),
				(bank >> 1) | (banks.chr[1] & (mask >> 1))
			); 
	
			chr.SwapBanks<SIZE_1K>
			( 
		     	0x1000U ^ swap,  
				bank | (banks.chr[2] & mask),
				bank | (banks.chr[3] & mask),
				bank | (banks.chr[4] & mask),
				bank | (banks.chr[5] & mask)
			); 
		}
	}
}
