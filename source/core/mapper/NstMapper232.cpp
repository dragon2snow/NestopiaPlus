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
#include "NstMapper232.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper232::SubReset(bool)
		{
			Map( 0x9000U,          &Mapper232::Poke_9000 );
			Map( 0xA000U, 0xFFFFU, &Mapper232::Poke_A000 );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(Mapper232,9000) 
		{ 
			data = data >> 1 & 0xC;
			prg.SwapBanks<SIZE_16K,0x0000U>( data | (prg.GetBank<SIZE_16K,0x0000U>() & 0x3), data | 0x3 );
		}

		NES_POKE(Mapper232,A000) 
		{ 
			prg.SwapBank<SIZE_16K,0x0000U>( (prg.GetBank<SIZE_16K,0x0000U>() & 0xC) | (data & 0x3) );
		}
	}
}
