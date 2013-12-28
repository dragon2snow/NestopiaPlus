////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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
#include "NstMapper051.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
		
		void Mapper51::SubReset(const bool hard)
		{
			if (hard)
			{
				bank = 0;
				mode = 1;
			}

			Map( WRK_PEEK );
			Map( 0x6000U, 0x7FFFU, &Mapper51::Poke_6000 );
			Map( 0x8000U, 0xBFFFU, &Mapper51::Poke_8000 );
			Map( 0xC000U, 0xDFFFU, &Mapper51::Poke_C000 );
			Map( 0xE000U, 0xFFFFU, &Mapper51::Poke_8000 );
		}

		void Mapper51::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					{
						const uint data = state.Read8();
						mode = data & 0x3;
						bank = data >> 4;
					}

					UpdateBanks();
				}
	
				state.End();
			}
		}
	
		void Mapper51::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( mode | (bank << 4) ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		void Mapper51::UpdateBanks()
		{
			uint offset;

			if (mode & 0x1)
			{
				prg.SwapBank<NES_32K,0x0000U>( bank );
				offset = 0x23;
			}
			else
			{
				prg.SwapBanks<NES_16K,0x0000U>( (bank << 1) | (mode >> 1), (bank << 1) | 0x7 );
				offset = 0x2F;
			}

			wrk.SwapBank<NES_8K,0x0000U>( offset | (bank << 2) );
			ppu.SetMirroring( (mode == 0x3) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}

		NES_POKE(Mapper51,6000) 
		{
			mode = ((data & 0x10) >> 3) | ((data & 0x02) >> 1);	
			UpdateBanks();
		}
	
		NES_POKE(Mapper51,8000) 
		{
			bank = data & 0x0F;
			UpdateBanks();
		}
	
		NES_POKE(Mapper51,C000)
		{
			bank = data & 0x0F;
			mode = (mode & 0x01) | ((data & 0x10) >> 3);
			UpdateBanks();
		}
	}
}
