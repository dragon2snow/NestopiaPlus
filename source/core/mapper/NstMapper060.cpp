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
#include "NstMapper060.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper60::SubReset(const bool hard)
		{
			if (pRomCrc == 0xF9C484A0UL)
			{
				// Reset-triggered 4-in-1
	
				if (hard)
					game = 0;
				else
					game = (game + 1) & 0x3;
	
				chr.SwapBank<SIZE_8K,0x0000U>( game );
				prg.SwapBanks<SIZE_16K,0x0000U>( game, game );
			}
			else
			{
				if (hard)
					prg.SwapBank<SIZE_16K,0x4000U>( 1 );

				Map( 0x8000U, 0xFFFFU, &Mapper60::Poke_Prg );			
			}
		}
	
		void Mapper60::SubLoad(State::Loader& state)
		{
			if (pRomCrc == 0xF9C484A0UL)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						game = state.Read8() & 0x3;
	
					state.End();
				}
			}
		}
	
		void Mapper60::SubSave(State::Saver& state) const
		{
			if (pRomCrc == 0xF9C484A0UL)
				state.Begin('R','E','G','\0').Write8( game ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper60,Prg) 
		{
			ppu.SetMirroring( (address & 0x8) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
	
			if (address & 0x80)
				prg.SwapBanks<SIZE_16K,0x0000U>( address >> 4 & 0x7, address >> 4 & 0x7 );
			else
				prg.SwapBank<SIZE_32K,0x0000U>( address >> 5 & 0x3 );
	
			chr.SwapBank<SIZE_8K,0x0000U>( address & 0x07 );
		}
	}
}
