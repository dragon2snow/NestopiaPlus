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
#include "NstMapper046.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper46::SubReset(bool)
		{
			Map( 0x6000U, 0x7FFFU, &Mapper46::Poke_6000 );
			Map( 0x8000U, 0xFFFFU, &Mapper46::Poke_8000 );
		}
		
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper46,6000) 
		{
			ppu.Update();
			prg.SwapBank<SIZE_32K,0x0000U>( (prg.GetBank<SIZE_32K,0x0000U>() & 0x1) | ((data << 1) & 0x1E) );
			chr.SwapBank<SIZE_8K,0x0000U>(  (chr.GetBank<SIZE_8K,0x0000U>()  & 0x7) | ((data >> 1) & 0x78) );
		}
	
		NES_POKE(Mapper46,8000) 
		{
			ppu.Update();
			prg.SwapBank<SIZE_32K,0x0000U>( ((data >> 0) & 0x1) | (prg.GetBank<SIZE_32K,0x0000U>() & 0x1E) );
			chr.SwapBank<SIZE_8K,0x0000U>(  ((data >> 4) & 0x7) | (chr.GetBank<SIZE_8K,0x0000U>()  & 0x78) );
		}
	}
}
