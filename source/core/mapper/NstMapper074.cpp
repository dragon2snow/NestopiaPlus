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

#include "../NstMapper.hpp"
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper074.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper74::Mapper74(Context& c)
		: 
		Mmc3(c,c.cRom.Size() ? CRAM_4K : 0), 
		GetChrType
		(
	     	c.pRomCrc == 0xC856F188UL  ? GetChrType3 : // Crystalis (Chinese)
			c.pRomCrc == 0x1A13BA25UL  ? GetChrType3 : // Captain Tsubasa Vol 2 - Super Striker (Chinese)
			c.pRomCrc == 0x1EE6D43BUL  ? GetChrType4 : // Young Chivalry (Chinese)
			c.pRomCrc == 0x9767DC74UL  ? GetChrType4 : // Ying Lie Qun Xia Zhuan (Chinese)
			c.pRom.Size() == SIZE_512K ? GetChrType1 : 
			                             GetChrType2
		) 
		{}
	
		void Mapper74::SubReset(bool hard)
		{
			Mmc3::SubReset( hard );

			if (hard)
				std::memset( ram, 0, sizeof(ram) );

			Map( 0x5000U, 0x5FFFU, &Mapper74::Peek_Ram, &Mapper74::Poke_Ram );
		}
	
		void Mapper74::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','A','M','\0'))
					state.Uncompress( ram );

				state.End();
			}
		}

		void Mapper74::SubSave(State::Saver& state) const
		{
			state.Begin('R','A','M','\0').Compress( ram ).End();
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		uint Mapper74::GetChrType1(const uint bank)
		{
			return bank == 8 || bank == 9;
		}
	
		uint Mapper74::GetChrType2(const uint bank)
		{
			return bank <= 1;
		}

		uint Mapper74::GetChrType3(const uint bank)
		{
			return bank <= 3;
		}

		uint Mapper74::GetChrType4(const uint bank)
		{
			return bank >= 8 && bank <= 11;
		}

		void Mapper74::SwapChr(const uint address,const uint bank) const
		{
			chr.Source( GetChrType(bank) ).SwapBank<SIZE_1K>( address, bank ); 
		}
	
		void Mapper74::UpdateChr() const
		{
			ppu.Update();
	
			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;
	
			SwapChr( 0x0000U ^ swap, (banks.chr[0] << 1) | 0 ); 
			SwapChr( 0x0400U ^ swap, (banks.chr[0] << 1) | 1 ); 
			SwapChr( 0x0800U ^ swap, (banks.chr[1] << 1) | 0 ); 
			SwapChr( 0x0C00U ^ swap, (banks.chr[1] << 1) | 1 ); 
			SwapChr( 0x1000U ^ swap,  banks.chr[2]           ); 
			SwapChr( 0x1400U ^ swap,  banks.chr[3]           ); 
			SwapChr( 0x1800U ^ swap,  banks.chr[4]           ); 
			SwapChr( 0x1C00U ^ swap,  banks.chr[5]           ); 
		}

		NES_PEEK(Mapper74,Ram)
		{
			return ram[address-0x5000];
		}

		NES_POKE(Mapper74,Ram)
		{
			ram[address-0x5000] = data;
		}
	}
}
