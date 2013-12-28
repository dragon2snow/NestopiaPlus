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
#include "NstMapper251.hpp"
	   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper251::SubReset(const bool hard)
		{
			exBanks[3] = exBanks[2] = exBanks[1] = exBanks[0] = 0;
			exCount = exEnabled = 0;

			Mmc3::SubReset( hard );

			Map( 0x6000U, 0x7FFFU, &Mapper251::Poke_6000 );

			for (uint i=0xA001U; i < 0xC000U; i += 0x2)
				Map( i, &Mapper251::Poke_A001 );

			banks.prg[0] = 0x30;
			banks.prg[1] = 0x31;
			UpdatePrg();
	//		prg.SwapBanks<SIZE_8K,0x0000U>(0x30,0x31,0x3E,0x3F);
		}
	
		void Mapper251::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<5> data( state );

					for (uint i=0; i < 4; ++i)
						exBanks[i] = data[i];

					exEnabled = data[4] & 0x80;
					exCount = data[4] & 0x03;
				}
	
				state.End();
			}
		}
	
		void Mapper251::SubSave(State::Saver& state) const
		{
			const u8 data[] = 
			{
				exBanks[0],
				exBanks[1],
				exBanks[2],
				exBanks[3],
				exEnabled | exCount,
			};

			state.Begin('R','E','G','\0').Write( data ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		void Mapper251::UpdatePrg()
		{
			const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

			prg.SwapBanks<SIZE_8K,0x0000U>
			( 
     			((banks.prg[i]   & ((exBanks[3] & 0x3F) ^ 0x3F)) | exBanks[1]), 
				((banks.prg[1]   & ((exBanks[3] & 0x3F) ^ 0x3F)) | exBanks[1]), 
				((banks.prg[i^2] & ((exBanks[3] & 0x3F) ^ 0x3F)) | exBanks[1]) & 0xFE, 
				((banks.prg[3]   & ((exBanks[3] & 0x3F) ^ 0x3F)) | exBanks[1])
			);
		}

		void Mapper251::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;			
			const uint exChr[2] = { exBanks[1] << 4, exBanks[2] << 4 | 0xF };

			chr.SwapBanks<SIZE_2K>
			( 
		     	0x0000U ^ swap, 
				(banks.chr[0] | (exChr[0] >> 1)) & (exChr[1] >> 1), 
				(banks.chr[1] | (exChr[0] >> 1)) & (exChr[1] >> 1)
			); 

			chr.SwapBanks<SIZE_1K>
			( 
		     	0x1000U ^ swap, 
				(banks.chr[2] | exChr[0]) & exChr[1], 
				(banks.chr[3] | exChr[0]) & exChr[1], 
				(banks.chr[4] | exChr[0]) & exChr[1], 
				(banks.chr[5] | exChr[0]) & exChr[1] 
			); 
		}
	
		NES_POKE(Mapper251,6000)
		{
			if (exEnabled)
			{
				exBanks[exCount] = data;

				if (++exCount == 4)
				{
					exCount = 0;
					UpdatePrg();
					UpdateChr();
				}
			}
		}

		NES_POKE(Mapper251,A001)
		{
			exEnabled = data & 0x80;

			if (exEnabled)
				exCount = 0;
		}
	}
}
