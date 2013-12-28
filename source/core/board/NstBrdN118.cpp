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
#include "NstBrdN118.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void N118::SubReset(const bool hard)
			{
				if (hard)
					reg = 0;

				for (dword i=0x8000U; i <= 0xFFFFU; i += 0x2)
				{
					Map( i + 0x0, &N118::Poke_8000 );
					Map( i + 0x1, &N118::Poke_8001 );
				}
			}
		
			void N118::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						reg = state.Read8();
		
					state.End();
				}
			}
		
			void N118::SaveState(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( reg ).End();
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_POKE(N118,8000)
			{
				reg = data;

				if (type == TYPE_MIRROR_CTRL)
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_ONE : Ppu::NMT_ZERO );
			}
		
			NES_POKE(N118,8001)
			{
				const uint index = reg & 0x7;
		
				if (index < 0x6)
				{
					ppu.Update();
		
					if (index > 0x1)
					{
						chr.SwapBank<NES_1K>( (index << 10) + 0x800U, data | 0x40 );
					}
					else
					{
						chr.SwapBank<NES_2K>( index << 11, data >> 1 );
					}
				}
				else 
				{
					prg.SwapBank<NES_8K>( (index - 0x6) << 13, data );
				}
			}
		}
	}
}
