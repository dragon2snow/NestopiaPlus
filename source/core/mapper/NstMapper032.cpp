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
#include "NstMapper032.hpp"
		   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper32::SubReset(const bool hard)
		{
			if (hard)
			{
				pRomOffset = 0x0000U;

				if (pRomCrc == 0xC0FED437UL) // Major League
					ppu.SetMirroring( Ppu::NMT_ZERO );
			}

			Map( 0x8000U, 0x8FFFU, &Mapper32::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper32::Poke_9000 );			
			Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );
	
			for (uint i=0xB000U; i < 0xC000U; i += 0x8)
			{
				Map( i + 0x0, CHR_SWAP_1K_0 );
				Map( i + 0x1, CHR_SWAP_1K_1 );
				Map( i + 0x2, CHR_SWAP_1K_2 );
				Map( i + 0x3, CHR_SWAP_1K_3 );
				Map( i + 0x4, CHR_SWAP_1K_4 );
				Map( i + 0x5, CHR_SWAP_1K_5 );
				Map( i + 0x6, &Mapper32::Poke_B006 );
				Map( i + 0x7, &Mapper32::Poke_B007 );
			}
		}
	
		void Mapper32::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					pRomOffset = (state.Read8() & 0x2) << 13;
	
				state.End();
			}
		}
	
		void Mapper32::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( pRomOffset >> 13 ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper32,8000)
		{
			prg.SwapBank<SIZE_8K>( pRomOffset, data );
		}
	
		NES_POKE(Mapper32,9000)
		{
			ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
			pRomOffset = (data & 0x2) << 13;
		}
	
		NES_POKE(Mapper32,B006) 
		{ 
			ppu.Update(); 
			chr.SwapBank<SIZE_1K,0x1800U>(data); 
	
			if (pRomCrc == 0xC0FED437UL && (data & 0x40)) // Major League
			{
				const uchar mirroring[4] = {0,0,0,1};
				ppu.SetMirroring( mirroring );
			}
		}
	
		NES_POKE(Mapper32,B007) 
		{ 
			ppu.Update(); 
			chr.SwapBank<SIZE_1K,0x1C00U>(data); 
	
			if (pRomCrc == 0xC0FED437UL && (data & 0x40)) // Major League
				ppu.SetMirroring( Ppu::NMT_ZERO );
		}
	}
}
