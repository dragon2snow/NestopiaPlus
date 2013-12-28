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
#include "NstMapper189.hpp"
	   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper189::SubReset(const bool hard)
		{
			Mmc3::SubReset( hard );
	
			for (uint i=0x0000U; i < 0x1000U; i += 0x200)
			{
				Map( 0x4100U + i, 0x41FFU + i, &Mapper189::Poke_4100 );
				Map( 0x6100U + i, 0x61FFU + i, &Mapper189::Poke_6100 );
			}

			for (uint i=0x8000U; i < 0xA000U; i += 0x2)
			{
				Map( i + 0x0, &Mapper189::Poke_8000 );
				Map( i + 0x1, &Mapper189::Poke_8001 );
			}
	
			if (hard)
				prg.SwapBank<NES_32K,0x0000U>(0);
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper189,4100)
		{
			prg.SwapBank<NES_32K,0x0000U>( data >> 4 );
		}
	
		NES_POKE(Mapper189,6100)
		{
			prg.SwapBank<NES_32K,0x0000U>( data & 0x3 );
		}
	
		NES_POKE(Mapper189,8000)
		{
			regs.ctrl0 = data & Regs::CTRL0_MODE;
		}
	
		NES_POKE(Mapper189,8001)
		{
			ppu.Update();
	
			switch (regs.ctrl0)
			{
				case 0: chr.SwapBank<NES_2K,0x0000U>(data >> 1); break;
				case 1: chr.SwapBank<NES_2K,0x0800U>(data >> 1); break;
				case 2: chr.SwapBank<NES_1K,0x1000U>(data);      break;
				case 3: chr.SwapBank<NES_1K,0x1400U>(data);      break;
				case 4: chr.SwapBank<NES_1K,0x1800U>(data);      break;
				case 5: chr.SwapBank<NES_1K,0x1C00U>(data);      break;
			}
		}
	}
}
