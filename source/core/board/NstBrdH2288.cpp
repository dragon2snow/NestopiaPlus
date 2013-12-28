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
#include "NstBrdMmc3.hpp"
#include "NstBrdH2288.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			void H2288::SubReset(const bool hard)
			{
				if (hard)
				{
					exRegs[0] = 0;
					exRegs[1] = 0;
				}

				Mmc3::SubReset( hard );

				Map( 0x5000U, 0x57FFU, &H2288::Peek_5000 );
				Map( 0x5800U, 0x5FFFU, &H2288::Peek_5000, &H2288::Poke_5800 );

				for (uint i=0x8000U; i < 0x9000U; i += 0x2)
					Map( i, &H2288::Poke_8000 );
			}
		
			void H2288::SubLoad(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					{
						const State::Loader::Data<2> data( state );

						exRegs[0] = data[0];
						exRegs[1] = data[1];
					}

					state.End();
				}
			}

			void H2288::SubSave(State::Saver& state) const
			{
				const u8 data[2] = 
				{
					exRegs[0],
					exRegs[1]
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void H2288::UpdatePrg()
			{
				if (!(exRegs[0] & 0x40))
					Mmc3::UpdatePrg();
			}

			NES_PEEK(H2288,5000)
			{
				return ((address >> 8) & 0xFE) | ((((address & 0x1) ^ 0x1) & ((address >> 8) & 0x1)) ^ 0x1);
			}

			NES_POKE(H2288,5800) 
			{ 
				address &= 0x1;

				if (exRegs[address] != data)
				{
					exRegs[address] = data;					

					if (exRegs[0] & 0x40)
					{
						data = (exRegs[0] & 0x05) | ((exRegs[0] & 0x08) >> 2) | ((exRegs[0] & 0x20) >> 2);
						address = (exRegs[0] & 0x2) >> 1;

						prg.SwapBanks<SIZE_16K,0x0000U>( data & ~address, data | address );
					}
					else
					{
						Mmc3::UpdatePrg();
					}
				}
			}

			NES_POKE(H2288,8000)
			{ 
				static const u8 security[8] = {0,3,1,5,6,7,2,4};
				NES_CALL_POKE( Mmc3, 8000, address, (data & 0xC0) | security[data & 0x07] );
			}
		}
	}
}
