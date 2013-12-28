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
#include "NstMapper227.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper227::SubReset(const bool hard)
		{
			NES_CALL_POKE(Mapper227,Prg,0x8000U,0x00);
			Map( 0x8000U, 0xFFFFU, &Mapper227::Poke_Prg );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper227,Prg) 
		{
			const uint bank = 
			(
				((address & 0x0100) >> 4) | 
				((address & 0x0078) >> 3)
			);
	
			if (address & 0x1)
			{
				prg.SwapBank<SIZE_32K,0x0000U>( bank );
			}
			else 
			{
				const uint offset = ((address >> 2) & 0x1) + (bank << 1);
				prg.SwapBanks<SIZE_16K,0x0000U>( offset, offset );
			}
	
			if (!(address & 0x80))
				prg.SwapBank<SIZE_16K,0x4000U>( ((address & 0x200) ? 0x7 : 0x0) + ((bank & 0x1C) << 1) );
	
			ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}
	}
}
