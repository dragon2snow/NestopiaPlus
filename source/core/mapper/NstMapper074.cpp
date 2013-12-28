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
		Mmc3 (c,WRAM_8K|CRAM_2K), 
		GetChrType 
		(
         	c.pRomCrc == 0x37AE04A8 ? GetChrType3 : // Sugoro Quest - Dice no Senshitachi
         	c.pRomCrc == 0xEAE675EA ? GetChrType2 : // Dai-2-Ji - Super Robot Taisen
		                              GetChrType1	// rest of 512k PRG-ROM carts
		) 
		{
		}
	
		void Mapper74::SubReset(bool hard)
		{
			Mmc3::SubReset( hard );
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
			return bank == 0;
		}
	
		uint Mapper74::GetChrType3(const uint bank)
		{
			return bank >= 128;
		}
	
		void Mapper74::SwapChr(const uint address,const uint bank) const
		{
			chr.Source( GetChrType( bank ) ).SwapBank<NES_1K>( address, bank ); 
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
	}
}
