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
#include "NstMapper156.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper156::SubReset(bool)
		{
			Map( 0xC000U, 0xC003U, &Mapper156::Poke_C000 );
			Map( 0xC008U, 0xC00BU, &Mapper156::Poke_C008 );
			Map( 0xC010U, PRG_SWAP_16K );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper156,C000) 
		{ 
			ppu.Update(); 
			chr.SwapBank<NES_1K>( 0x0000U + (address & 0x3) * NES_1K, data ); 
		}

		NES_POKE(Mapper156,C008) 
		{ 
			ppu.Update(); 
			chr.SwapBank<NES_1K>( 0x1000U + (address & 0x3) * NES_1K, data ); 
		}
	}
}
