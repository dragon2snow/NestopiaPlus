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
#include "NstBrdA65AS.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void A65AS::SubReset(const bool hard)
			{
				Map( 0x8000U, 0xFFFFU, &A65AS::Poke_Prg );
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			NES_POKE(A65AS,Prg) 
			{ 
				if (data & 0x40)
					prg.SwapBank<SIZE_32K,0x0000U>( data >> 1 & 0xF );
				else
					prg.SwapBanks<SIZE_16K,0x0000U>( (data >> 1 & 0x18) | (data & 0x7), (data >> 1 & 0x18) | 0x7 );

				ppu.SetMirroring
				( 
			     	(data & 0x80) ? (data & 0x20) ? Ppu::NMT_ONE : Ppu::NMT_ZERO : 
									(data & 0x08) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL
				);
			}
		}
	}
}
