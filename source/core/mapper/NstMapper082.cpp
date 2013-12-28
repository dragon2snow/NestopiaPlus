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
#include "NstMapper082.hpp"
	 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper82::SubReset(const bool hard)
		{
			if (hard)
				swapOffset = 0x0000U;

			Map( 0x7EF0U, &Mapper82::Poke_7EF0 );
			Map( 0x7EF1U, &Mapper82::Poke_7EF1 );
			Map( 0x7EF2U, &Mapper82::Poke_7EF2 );
			Map( 0x7EF3U, &Mapper82::Poke_7EF3 );
			Map( 0x7EF4U, &Mapper82::Poke_7EF4 );
			Map( 0x7EF5U, &Mapper82::Poke_7EF5 );
			Map( 0x7EF6U, &Mapper82::Poke_7EF6 );
			Map( 0x7EFAU, &Mapper82::Poke_7EFA );
			Map( 0x7EFBU, &Mapper82::Poke_7EFB );
			Map( 0x7EFCU, &Mapper82::Poke_7EFC );
		}
	
		void Mapper82::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					swapOffset = (state.Read8() & 0x2) << 11;
	
				state.End();
			}
		}
	
		void Mapper82::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( swapOffset >> 11 ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper82,7EF0)
		{
			ppu.Update();	
			chr.SwapBank<SIZE_2K>( swapOffset | 0x0000U, data >> 1 );
		}
	
		NES_POKE(Mapper82,7EF1)
		{
			ppu.Update();
			chr.SwapBank<SIZE_2K>( swapOffset | 0x0800U, data >> 1 );
		}
	
		NES_POKE(Mapper82,7EF2) 
		{ 
			ppu.Update(); 
			chr.SwapBank<SIZE_1K>( swapOffset ^ 0x1000U, data );
		}
	
		NES_POKE(Mapper82,7EF3) 
		{ 
			ppu.Update(); 
			chr.SwapBank<SIZE_1K>( swapOffset ^ 0x1400U, data );
		}
	
		NES_POKE(Mapper82,7EF4) 
		{ 
			ppu.Update(); 
			chr.SwapBank<SIZE_1K>( swapOffset ^ 0x1800U, data );
		}
	
		NES_POKE(Mapper82,7EF5) 
		{ 
			ppu.Update(); 
			chr.SwapBank<SIZE_1K>( swapOffset ^ 0x1C00U, data );
		}
	
		NES_POKE(Mapper82,7EF6)
		{
			swapOffset = (data & 0x2) << 11;
			ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_VERTICAL : Ppu::NMT_HORIZONTAL );
		}
	
		NES_POKE(Mapper82,7EFA) { prg.SwapBank<SIZE_8K,0x0000U>(data >> 2); }
		NES_POKE(Mapper82,7EFB) { prg.SwapBank<SIZE_8K,0x2000U>(data >> 2); }
		NES_POKE(Mapper82,7EFC) { prg.SwapBank<SIZE_8K,0x4000U>(data >> 2); }
	}
}
