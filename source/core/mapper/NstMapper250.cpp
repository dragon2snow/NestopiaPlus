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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper250.hpp"
	   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper250::SubReset(const bool hard)
		{
			Mmc3::SubReset( hard );
	
			Map( 0x8000U, 0xFFFFU, &Mapper250::Poke_Prg );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper250,Prg)
		{
			data = address & 0xFF;
			address = (address & 0xE000U) | (address >> 10 & 0x1);
	
			switch (address)
			{
				case 0x8000U: NES_CALL_POKE( Mmc3, 8000,   address, data ); break;
				case 0x8001U: NES_CALL_POKE( Mmc3, 8001,   address, data ); break;	
				case 0xA000U: NES_CALL_POKE( Mmc3, Nmt_Hv, address, data ); break;
				case 0xA001U: NES_CALL_POKE( Mmc3, A001,   address, data ); break;
				case 0xC000U: NES_CALL_POKE( Mmc3, C000,   address, data ); break;
				case 0xC001U: NES_CALL_POKE( Mmc3, C001,   address, data ); break;
				case 0xE000U: NES_CALL_POKE( Mmc3, E000,   address, data ); break;
				case 0xE001U: NES_CALL_POKE( Mmc3, E001,   address, data ); break;
			}
		}
	}
}
