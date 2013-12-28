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
#include "NstMapper160.hpp"
			
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper160::SubReset(const bool hard)
		{
			if (hard)
				mode = 0x00;

			Map( 0x8000U, PRG_SWAP_8K_0 );
			Map( 0x8001U, PRG_SWAP_8K_1 );
			Map( 0x8002U, PRG_SWAP_8K_2 );
	
			Map( 0x9000U,          &Mapper160::Poke_9000 );
			Map( 0x9001U,          &Mapper160::Poke_9001 );
			Map( 0x9002U, 0x9007U, &Mapper160::Poke_9000 );
			Map( 0xD000U, 0xFFFFU, &Mapper160::Poke_D000 );
		}
	
		void Mapper160::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					mode = state.Read8();
	
				state.End();
			}
		}
	
		void Mapper160::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( mode ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper160,9000)
		{
			if (mode == 1)
			{
				ppu.Update();
				chr.SwapBank<SIZE_1K>( (address & 0x7) * 1024, data );
			}
		}
	
		NES_POKE(Mapper160,9001)
		{
			ppu.Update();
	
			switch (mode)
			{
				case 4:
		
					chr.SwapBanks<SIZE_4K,0x0000U>( data >> 1, data >> 1 );
					break;
		
				case 1:
		
					chr.SwapBank<SIZE_1K>( (0x9001U & 0x7) * 1024, data );
					break;
			}
		}
	
		NES_POKE(Mapper160,D000)
		{
			mode = data;
		}
	}
}
