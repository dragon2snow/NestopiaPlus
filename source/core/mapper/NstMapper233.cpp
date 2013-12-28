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
			Map( 0x8000U, 0xFFFFU, &Mapper233::Poke_Prg );

			if (hard)
				games = 0x00;
			else
				games ^= 0x20;

			NES_CALL_POKE(Mapper233,Prg,0x8000U,0x00);
		}
	
		void Mapper233::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( games >> 5 ).End();
		}

		void Mapper233::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					games = (state.Read8() & 0x1) << 5;

				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper233,Prg)
		{
			const uint bank = data & 0x1F;
	
			if (data & 0x20) 
				prg.SwapBanks<SIZE_16K,0x0000U>( games | bank, games | bank );
			else
				prg.SwapBank<SIZE_32K,0x0000U>( games >> 1 | bank >> 1 );
	
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
