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
#include "NstBrdVrc4.hpp"
#include "NstBrdVrc7.hpp"
	   
namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			Vrc7::Vrc7(Context& c)
			: Mapper(c), irq(c.cpu) {}

			void Vrc7::SubReset(const bool hard)
			{
				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );
		
				for (dword i=0x8000U; i <= 0xFFFFUL; ++i)
				{
					switch (i & 0xF038U)
					{
						case 0x8000U: Map( i, PRG_SWAP_8K_0    ); break;
						case 0x8008U:						
						case 0x8010U: Map( i, PRG_SWAP_8K_1    ); break;
						case 0x9000U: Map( i, PRG_SWAP_8K_2    ); break;
						case 0x9010U: 
						case 0x9030U: Map( i, &Vrc7::Poke_9010 ); break;
						case 0xA000U: Map( i, CHR_SWAP_1K_0    ); break;
						case 0xA008U:						
						case 0xA010U: Map( i, CHR_SWAP_1K_1    ); break;
						case 0xB000U: Map( i, CHR_SWAP_1K_2    ); break;
						case 0xB008U:						
						case 0xB010U: Map( i, CHR_SWAP_1K_3    ); break;
						case 0xC000U: Map( i, CHR_SWAP_1K_4    ); break;
						case 0xC008U:						
						case 0xC010U: Map( i, CHR_SWAP_1K_5    ); break;
						case 0xD000U: Map( i, CHR_SWAP_1K_6    ); break;
						case 0xD008U:						
						case 0xD010U: Map( i, CHR_SWAP_1K_7    ); break;
						case 0xE000U: Map( i, NMT_SWAP_VH01    ); break;
						case 0xE008U: 
						case 0xE010U: Map( i, &Vrc7::Poke_E008 ); break;
						case 0xF000U: Map( i, &Vrc7::Poke_F000 ); break;
						case 0xF008U: 
						case 0xF010U: Map( i, &Vrc7::Poke_F008 ); break;
					}											   
				}
			}
		
			void Vrc7::LoadState(State::Loader& state)
			{	
				while (const dword chunk = state.Begin())
				{
					if (chunk == NES_STATE_CHUNK_ID('I','R','Q','\0'))
						irq.LoadState( State::Loader::Subset(state).Ref() );

					state.End();
				}
			}
		
			void Vrc7::SaveState(State::Saver& state) const
			{
				irq.SaveState( State::Saver::Subset(state,'I','R','Q','\0').Ref() );
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_POKE(Vrc7,9010) 
			{
			}
		
			NES_POKE(Vrc7,E008) 
			{ 
				irq.Update();
				irq.unit.latch = data;
			}
			
			NES_POKE(Vrc7,F000) 
			{ 
				irq.Toggle( data );
			}
			
			NES_POKE(Vrc7,F008) 
			{ 
				irq.Toggle();
			}

			void Vrc7::VSync()
			{
				irq.VSync();
			}		
		}
	}
}
