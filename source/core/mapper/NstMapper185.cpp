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
#include "NstMapper185.hpp"
	  
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Mapper185::Mapper185(Context& c)
		: 
		Mapper   (c,CRAM_1K), 
		spyVsSpy (c.pRomCrc == 0xB36457C7UL) // Spy vs Spy
		{}
	
		void Mapper185::SubReset(const bool hard)
		{
			if (hard)
			{
				if (prg.Source().Size() >= SIZE_32K)
					prg.SwapBank<SIZE_32K,0x0000U>(0);
				else
					prg.SwapBank<SIZE_16K,0x4000U>(0);

				chr.Source(1).Fill(0xFF);
				chr.Source(1).SwapBank<SIZE_8K,0x0000U>(0);
			}

			Map( 0x8000U, 0xFFFFU, spyVsSpy ? &Mapper185::Poke_Prg_2 : &Mapper185::Poke_Prg_1 );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper185,Prg_1)
		{
			ppu.Update();	
			chr.Source( (data & 0x3) == 0 ).SwapBank<SIZE_8K,0x0000U>( data & 0x1 );
		}
	
		NES_POKE(Mapper185,Prg_2)
		{
			ppu.Update();
			chr.Source( data != 0x21 ).SwapBank<SIZE_8K,0x0000U>( 0 );
		}
	}
}
