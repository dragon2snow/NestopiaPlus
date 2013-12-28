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
#include "NstMapper135.hpp"
		 
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		void Mapper135::SubReset(const bool hard)
		{
			if (hard)
				command = 0;

			for (uint i=0x4100U; i < 0x6000U; ++i)
			{
				switch (i & 0x4101U)
				{
    				case 0x4100U: Map( i, &Mapper135::Poke_4100 ); break;
					case 0x4101U: Map( i, &Mapper135::Poke_4101 ); break;
				}
			}
		}
	
		void Mapper135::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					command = state.Read8();
	
				state.End();
			}
		}
	
		void Mapper135::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( command ).End();
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(Mapper135,4100) 
		{
			command = data;
		}

		NES_POKE(Mapper135,4101)
		{
			const uint index = command & 0x7;
			data &= 0x7;

			if (index < 5)
			{
				ppu.Update();

				if (index < 4)
				{
					chr.SwapBank<NES_2K>
					( 
				     	index << 11, 
						(chr.GetBank<NES_2K,0x0000U>() & 0x70) | (data << 1) | (index & 0x1)
					);
				}
				else
				{
					data <<= 4;
	
					chr.SwapBanks<NES_2K,0x0000U>
					( 
		             	data | (chr.GetBank<NES_2K,0x0000U>() & 0x0F),
						data | (chr.GetBank<NES_2K,0x0800U>() & 0x0F),
						data | (chr.GetBank<NES_2K,0x1000U>() & 0x0F),
						data | (chr.GetBank<NES_2K,0x1800U>() & 0x0F)
					);
				}
			}
			else if (index == 5)
			{
				prg.SwapBank<NES_32K,0x0000U>( data );
			}
			else if (index == 7)
			{
				static const uchar lut[4] =
				{
					Ppu::NMT_ZERO,
					Ppu::NMT_HORIZONTAL,
					Ppu::NMT_VERTICAL,
					Ppu::NMT_ONE
				};

				ppu.SetMirroring( lut[data >> 1] );
			}
		}
	}
}
