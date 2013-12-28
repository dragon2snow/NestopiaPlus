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
#include "NstBrd8157.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void Unl8157::SubReset(bool)
			{
				Map( 0x8000U, 0xFFFFU, &Unl8157::Poke_Prg );
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_POKE(Unl8157,Prg) 
			{ 
				const uint base = (address & 0x0060) >> 2;
				prg.SwapBanks<SIZE_16K,0x0000U>( base | ((address & 0x001C) >> 2), base | ((address & 0x0200) ? 0x7 : 0x0) );
				ppu.SetMirroring( (address & 0x0002) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			}
		}
	}
}
