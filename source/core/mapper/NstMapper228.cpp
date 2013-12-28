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
#include "NstMapper228.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper228::SubReset(const bool hard)
		{
			Map( 0x8000U, 0xFFFFU, &Mapper228::Poke_Prg );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper228,Prg) 
		{
			uint pBank = ((address & 0x0780U) >> 7);
	
			switch ((address & 0x1800U) >> 11)
			{
		     	case 1: pBank |= 0x10; break;
         		case 3:	pBank |= 0x20; break;
			}
	
			if (address & 0x20)
			{
				pBank <<= 1;
	
				if (address & 0x40)
					++pBank;
	
				prg.SwapBanks<SIZE_16K,0x0000U>( pBank << 1, pBank << 1 );
			}
			else
			{
				prg.SwapBank<SIZE_32K,0x0000U>( pBank );
			}
	
			ppu.SetMirroring( (address & 0x2000U) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
	
			chr.SwapBank<SIZE_8K,0x0000U>( ((address & 0xF) << 2) | (data & 0x3) );
		}
	}
}
