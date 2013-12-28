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
#include "NstBrdCc21.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void Cc21::SubReset(bool)
			{
				Map( 0x8000U, 0xFFFFU, &Cc21::Poke_Prg );
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_POKE(Cc21,Prg) 
			{ 
				ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_ONE : Ppu::NMT_ZERO );
				chr.SwapBank<SIZE_8K,0x0000U>( address & 0x1 );
			}
		}
	}
}
