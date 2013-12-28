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
#include "NstMapper092.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper92::SubReset(bool)
		{
			Map( 0x8000U, 0x8FFFU, &Mapper92::Poke_8000 );
			Map( 0x9000U, 0xFFFFU, &Mapper92::Poke_9000 );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper92,8000)
		{
			switch (address & 0xF0)
			{
				case 0xB0: 
		
					prg.SwapBanks<NES_16K,0x0000U>( 0, address & 0xF ); 
					break;
		
				case 0x70: 
		
					ppu.Update();
					chr.SwapBank<NES_8K,0x0000U>( address & 0xF );
					break;
			}
		}
	
		NES_POKE(Mapper92,9000)
		{
			switch (address & 0xF0)
			{
				case 0xD0: 
		
					prg.SwapBanks<NES_16K,0x0000U>( 0, address & 0xF ); 
					break;
		
				case 0xE0: 
		
					ppu.Update();
					chr.SwapBank<NES_8K,0x0000U>( address & 0xF );
					break;
			}
		}
	}
}
