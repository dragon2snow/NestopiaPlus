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
#include "NstMapper015.hpp"
		   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper15::SubReset(bool)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper15::Poke_Prg );
			NES_CALL_POKE(Mapper15,Prg,0x8000U,0x00);
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper15,Prg)
		{
			uint bank = (data & 0x3F) << 1;
			uint flip = data >> 7;
			
			uint banks[4];
	
			switch (address & 0x3)
			{
         		case 0x0:
	
					bank &= 0x7C;
					banks[0] = bank | (0 ^ flip);
					banks[1] = bank | (1 ^ flip);
					banks[2] = bank | (2 ^ flip);
					banks[3] = bank | (3 ^ flip);
					break;
	
				case 0x1:
	
					banks[0] = bank | (0 ^ flip);
					banks[1] = bank | (1 ^ flip);
					banks[2] = 0x7E | (0 ^ flip);
					banks[3] = 0x7F | (1 ^ flip);
					break;
	
				case 0x2:
	
					bank ^= flip;
					banks[0] = bank;
					banks[1] = bank;
					banks[2] = bank;
					banks[3] = bank;
					break;
	
				case 0x3:
	
					banks[0] = bank | (0 ^ flip);
					banks[1] = bank | (1 ^ flip);
					banks[2] = bank | (0 ^ flip);
					banks[3] = bank | (1 ^ flip);
					break;
			}
	
			prg.SwapBanks<SIZE_8K,0x0000U>( banks[0], banks[1], banks[2], banks[3] );
			ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}
	}
}
