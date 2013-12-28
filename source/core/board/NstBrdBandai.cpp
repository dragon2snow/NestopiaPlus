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
#include "../NstClock.hpp"
#include "NstBrdBandai.hpp"
	   
namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			Bandai::Bandai(Context& c,const Type t)
			: Mapper(c), irq(c.cpu), type(t) {}
		
			void Bandai::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					latch = 0;
					count = 0;
				}
			}
		
			void Bandai::SubReset(const bool hard)
			{
				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

				for (dword i = (type == TYPE_B ? 0x8000U : 0x6000U); i <= 0xFFFFU; i += 0x10)
				{
					switch (type)
					{
						case TYPE_C:
		
							Map( i + 0xD, &Bandai::Poke_D );
		
						case TYPE_A:
		
							Map( i + 0x0, CHR_SWAP_1K_0 );
							Map( i + 0x1, CHR_SWAP_1K_1 );
							Map( i + 0x2, CHR_SWAP_1K_2 );
							Map( i + 0x3, CHR_SWAP_1K_3 );
							Map( i + 0x4, CHR_SWAP_1K_4 );
							Map( i + 0x5, CHR_SWAP_1K_5 );
							Map( i + 0x6, CHR_SWAP_1K_6 );
							Map( i + 0x7, CHR_SWAP_1K_7 );				
							Map( i + 0x8, PRG_SWAP_16K  );				
							break;
		
						case TYPE_B:
		
							Map( i + 0x0, &Bandai::Poke_0 );
							Map( i + 0x8, &Bandai::Poke_8 );
							break;
					}
		
					Map( i + 0x9, NMT_SWAP_VH01   );
					Map( i + 0xA, &Bandai::Poke_A );
					Map( i + 0xB, &Bandai::Poke_B );
					Map( i + 0xC, &Bandai::Poke_C );
				}
			}
		
			void Bandai::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
					{
						const State::Loader::Data<5> data( state );
		
						irq.EnableLine( data[0] & 0x1 );
						irq.unit.latch = data[1] | (data[2] << 8);
						irq.unit.count = data[3] | (data[4] << 8);
					}
		
					state.End();
				}
			}
		
			void Bandai::SaveState(State::Saver& state) const
			{				
				const u8 data[5] =
				{
					irq.IsLineEnabled() ? 0x1 : 0x0,
					irq.unit.latch & 0xFF,
					irq.unit.latch >> 8,
					irq.unit.count & 0xFF,
					irq.unit.count >> 8
				};

				state.Begin('I','R','Q','\0').Write( data ).End();
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_POKE(Bandai,0)
			{
				data = (data & 0x1) << 4;
				prg.SwapBanks<NES_16K,0x0000U>( data | (prg.GetBank<NES_16K,0x0000U>() & 0x0F), data | 0xF );
			}
		
			NES_POKE(Bandai,8)
			{
				prg.SwapBank<NES_16K,0x0000U>( (prg.GetBank<NES_16K,0x0000U>() & 0x10) | (data & 0x0F) );
			}
		
			NES_POKE(Bandai,A) 
			{ 
				irq.Update();
				irq.unit.count = irq.unit.latch;			
				irq.EnableLine( data & 0x1 );
				irq.ClearIRQ();
			}
		
			NES_POKE(Bandai,B) 
			{ 
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0xFF00U) | data;
			}
		
			NES_POKE(Bandai,C) 
			{ 
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0x00FFU) | (data << 8);
			}
		
			NES_POKE(Bandai,D)
			{
			}
		
			ibool Bandai::Irq::Signal()
			{
				count = (count - 1) & 0xFFFFU;
				return count == 0;
			}
		
			void Bandai::VSync()
			{
				irq.VSync();
			}
		}
	}
}
