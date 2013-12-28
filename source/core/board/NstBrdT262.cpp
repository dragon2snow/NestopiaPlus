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
#include "NstBrdT262.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void T262::SubReset(const bool hard)
			{
				mode = false;
				prg.SwapBanks<SIZE_16K,0x0000U>( 0x0, 0x07 );

				Map( 0x8000U, 0xFFFFU, &T262::Poke_Prg );
			}
		
			void T262::SubSave(State::Saver& state) const
			{
				state.Begin('R','E','G','\0').Write8( mode ).End();
			}

			void T262::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
						mode = state.Read8() & 0x1;

					state.End();
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			NES_POKE(T262,Prg) 
			{ 
				if (mode || address == 0x8000U)
				{
					prg.SwapBank<SIZE_16K,0x0000U>( (prg.GetBank<SIZE_16K,0x0000U>() & 0x38) | (data & 0x07) );
				}
				else
				{
					mode = true;
					data = (address >> 3 & 0x20) | (address >> 2 & 0x18);
					prg.SwapBanks<SIZE_16K,0x0000U>( data | (prg.GetBank<SIZE_16K,0x0000U>() & 0x07), data | 0x07 );
					ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
				}
			}
		}
	}
}
