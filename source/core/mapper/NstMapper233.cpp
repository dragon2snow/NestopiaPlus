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
#include "NstMapper233.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper233::SubReset(const bool hard)
		{
			if (hard)
				NES_CALL_POKE(Mapper233,Prg,0x8000U,0x00);

			Map( 0x8000U, 0xFFFFU, &Mapper233::Poke_Prg );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper233,Prg)
		{
			const uint bank = data & 0x1F;
	
			if (data & 0x20) 
				prg.SwapBanks<SIZE_16K,0x0000U>( bank, bank );
			else
				prg.SwapBank<SIZE_32K,0x0000U>( bank >> 1 );
	
			static const uchar lut[4][4] =
			{
				{0,0,0,1},
				{0,1,0,1},
				{0,0,1,1},
				{1,1,1,1}
			};
	
			ppu.SetMirroring( lut[data >> 6] );
		}
	}
}
